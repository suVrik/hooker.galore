#include "core/ecs/world.h"
#include "core/render/render_pass.h"
#include "core/resource/material.h"
#include "core/resource/texture.h"
#include "world/editor/editor_component.h"
#include "world/editor/guid_single_component.h"
#include "world/editor/preset_single_component.h"
#include "world/shared/level_single_component.h"
#include "world/shared/name_single_component.h"
#include "world/shared/render/material_component.h"
#include "world/shared/render/material_single_component.h"
#include "world/shared/render/model_component.h"
#include "world/shared/render/model_single_component.h"
#include "world/shared/render/texture_single_component.h"
#include "world/shared/resource_system.h"
#include "world/shared/window_single_component.h"

#define TINYGLTF_NO_STB_IMAGE       1
#define TINYGLTF_NO_STB_IMAGE_WRITE 1

#include <algorithm>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/file.h>
#include <entt/meta/factory.hpp>
#include <fmt/format.h>
#include <fstream>
#include <future>
#include <ghc/filesystem.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <mutex>
#include <SDL2/SDL_filesystem.h>
#include <tiny_gltf.h>
#include <unordered_set>
#include <yaml-cpp/yaml.h>

namespace hg {

namespace resource_system_details {

bool compare_case_insensitive(const std::string& a, const std::string& b) noexcept {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) {
        return tolower(a) == tolower(b);
    });
}

static std::mutex output_mutex;

template <typename T>
void iterate_recursive_parallel(const ghc::filesystem::path& directory, const std::string& extension, T callback) {
    std::vector<ghc::filesystem::path> files;
    if (ghc::filesystem::exists(directory)) {
        for (const auto& directory_entry : ghc::filesystem::recursive_directory_iterator(directory)) {
            const ghc::filesystem::path& file_path = directory_entry.path();
            if (compare_case_insensitive(file_path.extension().string(), extension)) {
                files.push_back(file_path);
            }
        }
    } else {
        throw std::runtime_error(fmt::format("Specified directory \"{}\" doesn't exist.", directory.string()));
    }

    std::mutex files_mutex;
    auto process_files = [&]() {
        while (true) {
            ghc::filesystem::path file;

            {
                std::lock_guard<std::mutex> guard(files_mutex);
                if (files.empty()) {
                    return;
                }
                file = files.back();
                files.pop_back();
            }

            callback(file);

            {
                std::lock_guard<std::mutex> guard(output_mutex);
                std::cout << "[RESOURCE] Loaded \"" << file.string() << "\"." << std::endl;
            }
        }
    };

    if (!files.empty()) {
        const size_t available_threads = std::thread::hardware_concurrency();
        const size_t background_threads = std::max(std::min(files.size(), available_threads), size_t(1)) - 1;

        std::vector<std::future<void>> futures;
        futures.reserve(background_threads);

        for (size_t i = 0; i < background_threads; i++) {
            futures.push_back(std::async(std::launch::async, process_files));
        }

        try {
            process_files();
        }
        catch (...) {
            // Wait until all futures are done first.
            for (std::future<void>& future : futures) {
                future.wait();
            }
            throw;
        }

        // Wait until all futures are done.
        for (std::future<void>& future : futures) {
            future.wait();
        }

        // Check whether any future had thrown an exception.
        for (std::future<void>& future : futures) {
            future.get();
        }
    }
}

void destroy_model_node(Model::Node& node) noexcept {
    for (Model::Node& child_node : node.children) {
        delete child_node.mesh;
        destroy_model_node(child_node);
    }
}

static const uint8_t RED_TEXTURE[4] = { 0xFF, 0x00, 0x00, 0xFF };

} // namespace resource_system_details

ResourceSystem::ResourceSystem(World& world)
        : NormalSystem(world)
        , m_model_observer(entt::observer(world, entt::collector.group<ModelComponent>()))
        , m_model_update_observer(entt::observer(world, entt::collector.replace<ModelComponent>()))
        , m_material_observer(entt::observer(world, entt::collector.group<MaterialComponent>()))
        , m_material_update_observer(entt::observer(world, entt::collector.replace<MaterialComponent>())) {
    load_textures();

    try {
        load_materials();
        load_models();

        try {
            load_presets();
            load_level();
        }
        catch (...) {
            auto &model_single_component = world.ctx<ModelSingleComponent>();
            for (auto&[material_name, material_ptr] : std::exchange(model_single_component.m_models, {})) {
                for (Model::Node &node : material_ptr->children) {
                    resource_system_details::destroy_model_node(node);
                }
            }

            throw;
        }
    }
    catch (...) {
        auto& texture_single_component = world.ctx<TextureSingleComponent>();
        texture_single_component.m_textures.clear();

        throw;
    }
}

ResourceSystem::~ResourceSystem() {
    auto& texture_single_component = world.ctx<TextureSingleComponent>();
    texture_single_component.m_textures.clear();
    texture_single_component.m_default_texture = Texture();

    auto& model_single_component = world.ctx<ModelSingleComponent>();
    for (auto& [material_name, material_ptr] : std::exchange(model_single_component.m_models, {})) {
        for (Model::Node& node : material_ptr->children) {
            resource_system_details::destroy_model_node(node);
        }
    }
}

void ResourceSystem::update(float /*elapsed_time*/) {
    auto& model_single_component = world.ctx<ModelSingleComponent>();

    auto model_updated = [&](const entt::entity entity) {
        auto& model_component = world.get<ModelComponent>(entity);
        if (!model_component.path.empty()) {
            const Model* original_model = model_single_component.get(model_component.path);
            if (original_model != nullptr) {
                model_component.model = *original_model;
            } else {
                const Model* blockout_model = model_single_component.get("blockout.glb");
                assert(blockout_model != nullptr);

                if (blockout_model != nullptr) {
                    model_component.path = "blockout.glb";
                    model_component.model = *blockout_model;
                }
            }
        }
    };

    m_model_observer.each(model_updated);
    m_model_update_observer.each(model_updated);

    auto& material_single_component = world.ctx<MaterialSingleComponent>();

    auto material_updated = [&](const entt::entity entity) {
        auto& material_component = world.get<MaterialComponent>(entity);
        if (!material_component.path.empty()) {
            const Material* original_material = material_single_component.get(material_component.path);
            if (original_material != nullptr) {
                material_component.material = original_material;
            } else {
                const Material* blockout_material = material_single_component.get("blockout_gray.yaml");
                assert(blockout_material != nullptr);

                if (blockout_material != nullptr) {
                    material_component.path = "blockout_gray.yaml";
                    material_component.material = blockout_material;
                }
            }
        }
    };

    m_material_observer.each(material_updated);
    m_material_update_observer.each(material_updated);
}

std::string ResourceSystem::get_resource_directory() const {
    char* const base_path = SDL_GetBasePath();
    if (base_path == nullptr) {
        throw std::runtime_error("Failed to get resource directory.");
    }

#if defined(__APPLE__) && defined(HG_MACOS_BUNDLE)
    const std::string result = base_path;
#else
    const std::string result = (ghc::filesystem::path(base_path) / "resources").string();
#endif

    SDL_free(base_path);

    return result;
}

void ResourceSystem::load_textures() const {
    auto& texture_single_component = world.set<TextureSingleComponent>();
    std::mutex texture_single_component_mutex;

    const ghc::filesystem::path directory = ghc::filesystem::path(get_resource_directory()) / "textures";
    resource_system_details::iterate_recursive_parallel(directory, ".dds", [&](const ghc::filesystem::path& file) {
        Texture texture = load_texture(file.string());
        if (bgfx::isValid(texture.handle)) {
            std::lock_guard<std::mutex> guard(texture_single_component_mutex);
            const std::string texture_name = file.lexically_relative(directory).lexically_normal().string();
            texture_single_component.m_textures.emplace(texture_name, std::move(texture));
        } else {
            std::lock_guard<std::mutex> guard(resource_system_details::output_mutex);
            std::cerr << "[RESOURCE] Failed to load texture \"" << file.string() << "\"." << std::endl;
        }
    });

    // Red square texture used as a fallback texture.
    const bgfx::Memory* memory = bgfx::makeRef(resource_system_details::RED_TEXTURE, std::size(resource_system_details::RED_TEXTURE));
    texture_single_component.m_default_texture.handle = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_NONE, memory);
    texture_single_component.m_default_texture.width = 1;
    texture_single_component.m_default_texture.height = 1;
    texture_single_component.m_default_texture.is_cube_map = false;

    if (!bgfx::isValid(texture_single_component.m_default_texture.handle)) {
        throw std::runtime_error("Failed to create a fallback texture!");
    }
}

Texture ResourceSystem::load_texture(const std::string &path) const noexcept {
    Texture result;
    bx::FileReader file_reader;
    if (bx::open(&file_reader, path.c_str())) {
        const bgfx::Memory* const memory = bgfx::alloc(static_cast<uint32_t>(bx::getSize(&file_reader)));
        if (bx::read(&file_reader, memory->data, static_cast<int32_t>(memory->size)) == static_cast<int32_t>(memory->size)) {
            bgfx::TextureInfo texture_info;
            result.handle = bgfx::createTexture(memory, BGFX_TEXTURE_NONE, 0, &texture_info);
            if (bgfx::isValid(result.handle)) {
                result.width = texture_info.width;
                result.height = texture_info.height;
                result.is_cube_map = texture_info.cubeMap;
                bgfx::setName(result.handle, path.c_str());
            }
        }
        bx::close(&file_reader);
    }
    return result;
}

void ResourceSystem::load_materials() const {
    auto& material_single_component = world.set<MaterialSingleComponent>();
    auto& texture_single_component = world.ctx<TextureSingleComponent>();
    std::mutex material_single_component_mutex;

    const ghc::filesystem::path directory = ghc::filesystem::path(get_resource_directory()) / "materials";
    resource_system_details::iterate_recursive_parallel(directory, ".yaml", [&](const ghc::filesystem::path& file) {
        std::unique_ptr<Material> material = std::make_unique<Material>();
        const std::string name = file.lexically_relative(directory).lexically_normal().string();

        load_material(texture_single_component, *material, file.string());

        std::lock_guard<std::mutex> guard(material_single_component_mutex);
        material_single_component.m_materials.emplace(name, std::move(material));
    });
}

void ResourceSystem::load_material(const TextureSingleComponent& texture_single_component, Material& result, const std::string &path) const {
    try {
        std::ifstream stream(path);
        if (!stream.is_open()) {
            throw std::runtime_error("Failed to open a file.");
        }

        YAML::Node node = YAML::Load(stream);
        if (!node.IsMap()) {
            throw std::runtime_error("Root node must be a map.");
        }

        YAML::Node color_roughness = node["color_roughness"];
        if (!color_roughness) {
            throw std::runtime_error("Field \"color_roughness\" is missing.");
        }

        const auto color_roughness_path = color_roughness.as<std::string>("");
        result.color_roughness = &texture_single_component.get(color_roughness_path);
        if (result.color_roughness == nullptr) {
            throw std::runtime_error(fmt::format("Specified texture \"{}\" doesn't exist.", color_roughness_path));
        }

        YAML::Node normal_metal_ao = node["normal_metal_ao"];
        if (!normal_metal_ao) {
            throw std::runtime_error("Field \"normal_metal_ao\" is missing.");
        }

        const auto normal_metal_ao_path = normal_metal_ao.as<std::string>("");
        result.normal_metal_ao = &texture_single_component.get(normal_metal_ao_path);
        if (result.normal_metal_ao == nullptr) {
            throw std::runtime_error(fmt::format("Specified texture \"{}\" doesn't exist.", normal_metal_ao_path));
        }

        YAML::Node parallax = node["parallax"];
        if (parallax) {
            const auto parallax_path = parallax.as<std::string>("");
            result.parallax = &texture_single_component.get(parallax_path);
            if (result.parallax == nullptr) {
                throw std::runtime_error(fmt::format("Specified texture \"{}\" doesn't exist.", parallax_path));
            }

            YAML::Node parallax_scale = node["parallax_scale"];
            if (!parallax_scale) {
                throw std::runtime_error("Field \"parallax_scale\" is missing.");
            }
            result.parallax_scale = parallax_scale.as<float>(0.f);
            if (result.parallax_scale <= -glm::epsilon<float>() || result.parallax_scale >= 1.f + glm::epsilon<float>()) {
                throw std::runtime_error("Invalid \"parallax_scale\" value.");
            }

            YAML::Node parallax_steps = node["parallax_steps"];
            if (!parallax_steps) {
                throw std::runtime_error("Field \"parallax_steps\" is missing.");
            }
            result.parallax_steps = parallax_steps.as<float>(0.f);
            if (result.parallax_steps <= -glm::epsilon<float>() || result.parallax_steps >= 32.f + glm::epsilon<float>()) {
                throw std::runtime_error("Invalid \"parallax_steps\" value.");
            }
        }
    }
    catch (const std::runtime_error& error) {
        throw std::runtime_error(fmt::format("Failed to load material \"{}\".\nDetails: {}", path, error.what()));
    }
}

void ResourceSystem::load_models() const  {
    auto& model_single_component = world.set<ModelSingleComponent>();
    std::mutex model_single_component_mutex;

    const ghc::filesystem::path directory = ghc::filesystem::path(get_resource_directory()) / "models";
    resource_system_details::iterate_recursive_parallel(directory, ".glb", [&](const ghc::filesystem::path& file) {
        std::unique_ptr<Model> model = std::make_unique<Model>();
        const std::string name = file.lexically_relative(directory).lexically_normal().string();

        load_model(*model, file.string());

        std::lock_guard<std::mutex> guard(model_single_component_mutex);
        model_single_component.m_models.emplace(name, std::move(model));
    });
}

void ResourceSystem::load_model(Model& result, const std::string &path) const {
    try {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string error, warning;
        if (loader.LoadBinaryFromFile(&model, &error, &warning, path)) {
            if (!warning.empty()) {
                std::cerr << "glTF warning: " << warning << std::endl;
            }

            const int scene_to_display = model.defaultScene > -1 ? model.defaultScene : 0;
            if (scene_to_display < model.scenes.size()) {
                const tinygltf::Scene &scene = model.scenes[scene_to_display];
                for (const int node_index : scene.nodes) {
                    Model::AABB node_bounds;

                    if (node_index >= model.nodes.size()) {
                        throw std::runtime_error("Invalid node index.");
                    }
                    load_model_node(result.children.emplace_back(), node_bounds, model, model.nodes[node_index]);

                    result.bounds.min_x = std::min(result.bounds.min_x, node_bounds.min_x);
                    result.bounds.min_y = std::min(result.bounds.min_y, node_bounds.min_y);
                    result.bounds.min_z = std::min(result.bounds.min_z, node_bounds.min_z);
                    result.bounds.max_x = std::max(result.bounds.max_x, node_bounds.max_x);
                    result.bounds.max_y = std::max(result.bounds.max_y, node_bounds.max_y);
                    result.bounds.max_z = std::max(result.bounds.max_z, node_bounds.max_z);
                }
            } else {
                throw std::runtime_error("Invalid defaultScene value.");
            }
        } else {
            throw std::runtime_error(error);
        }
    }
    catch (const std::runtime_error& error) {
        throw std::runtime_error(fmt::format("Failed to load model \"{}\".\nDetails: {}", path, error.what()));
    }
}

void ResourceSystem::load_model_node(Model::Node& result, Model::AABB& bounds, const tinygltf::Model &model, const tinygltf::Node &node) const {
    result.name = node.name;

    // Load node transform.
    if (node.matrix.size() == 16) {
        // Specified as 4x4 matrix.
        const glm::mat3 matrix(node.matrix[0],  node.matrix[1],  node.matrix[2],
                               node.matrix[4],  node.matrix[5],  node.matrix[6],
                               node.matrix[8],  node.matrix[9],  node.matrix[10]);
        result.translation = glm::vec3(node.matrix[12], node.matrix[13], node.matrix[14]);
        result.rotation = glm::quat(matrix);
        result.scale = glm::vec3(glm::length(matrix[0]), glm::length(matrix[1]), glm::length(matrix[2]));
    } else {
        // Specified as translation, rotation, scale (all optional).

        if (node.translation.size() == 3) {
            result.translation = glm::vec3(node.translation[0], node.translation[1], -node.translation[2]);
        } else {
            result.translation = glm::vec3(0.f);
        }

        if (node.rotation.size() == 4) {
            result.rotation = glm::quat(node.rotation[3], -node.rotation[0], -node.rotation[1], node.rotation[2]);
        } else {
            result.rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        }

        if (node.scale.size() == 3) {
            result.scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
        } else {
            result.scale = glm::vec3(1.f, 1.f, 1.f);
        }
    }

    if (node.mesh > -1 && node.mesh < model.meshes.size()) {
        result.mesh = new Model::Mesh();
        Model::AABB mesh_bounds;

        load_model_mesh(*result.mesh, mesh_bounds, model, node);

        bounds.min_x = std::min(bounds.min_x, mesh_bounds.min_x);
        bounds.min_y = std::min(bounds.min_y, mesh_bounds.min_y);
        bounds.min_z = std::min(bounds.min_z, mesh_bounds.min_z);
        bounds.max_x = std::max(bounds.max_x, mesh_bounds.max_x);
        bounds.max_y = std::max(bounds.max_y, mesh_bounds.max_y);
        bounds.max_z = std::max(bounds.max_z, mesh_bounds.max_z);
    }

    for (const int child_index : node.children) {
        if (child_index >= 0 && child_index < model.nodes.size()) {
            Model::AABB child_node_bounds;

            load_model_node(result.children.emplace_back(), child_node_bounds, model, model.nodes[child_index]);

            bounds.min_x = std::min(bounds.min_x, child_node_bounds.min_x);
            bounds.min_y = std::min(bounds.min_y, child_node_bounds.min_y);
            bounds.min_z = std::min(bounds.min_z, child_node_bounds.min_z);
            bounds.max_x = std::max(bounds.max_x, child_node_bounds.max_x);
            bounds.max_y = std::max(bounds.max_y, child_node_bounds.max_y);
            bounds.max_z = std::max(bounds.max_z, child_node_bounds.max_z);
        } else {
            throw std::runtime_error("Invalid child.");
        }
    }
}

void ResourceSystem::load_model_mesh(Model::Mesh& result, Model::AABB& bounds, const tinygltf::Model &model, const tinygltf::Node &node) const {
    const tinygltf::Mesh& mesh = model.meshes[node.mesh];
    for (const tinygltf::Primitive& primitive : mesh.primitives) {
        if (primitive.mode == TINYGLTF_MODE_TRIANGLES) {
            Model::AABB primitive_bounds;

            load_model_primitive(result.primitives.emplace_back(), primitive_bounds, model, primitive);

            bounds.min_x = std::min(bounds.min_x, primitive_bounds.min_x);
            bounds.min_y = std::min(bounds.min_y, primitive_bounds.min_y);
            bounds.min_z = std::min(bounds.min_z, primitive_bounds.min_z);
            bounds.max_x = std::max(bounds.max_x, primitive_bounds.max_x);
            bounds.max_y = std::max(bounds.max_y, primitive_bounds.max_y);
            bounds.max_z = std::max(bounds.max_z, primitive_bounds.max_z);
        }
    }
}

void ResourceSystem::load_model_primitive(Model::Primitive& result, Model::AABB& bounds, const tinygltf::Model &model, const tinygltf::Primitive& primitive) const {
    size_t num_vertices = 0;
    const bgfx::Memory* vertex_memory = nullptr;
    Model::BasicModelVertex* vertex_data = nullptr;

    int32_t attributes = 0;

    for (const auto& [attribute, accessor_index] : primitive.attributes) {
        if (accessor_index < 0 || accessor_index >= model.accessors.size()) {
            throw std::runtime_error("Invalid property accessor.");
        }

        const tinygltf::Accessor& accessor = model.accessors[accessor_index];
        if (accessor.sparse.isSparse ||
            accessor.count == 0 ||
            accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size()) {
            throw std::runtime_error("Invalid property accessor.");
        }

        const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
        if (buffer_view.target != TINYGLTF_TARGET_ARRAY_BUFFER ||
            buffer_view.buffer < 0 || buffer_view.buffer >= model.buffers.size()) {
            throw std::runtime_error("Invalid property accessor.");
        }

        const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
        if (accessor.byteOffset + buffer_view.byteOffset + buffer_view.byteLength > buffer.data.size() ||
            buffer_view.byteStride * (accessor.count - 1) >= buffer_view.byteLength) {
            throw std::runtime_error("Invalid property accessor.");
        }

        const uint8_t* const buffer_data = buffer.data.data() + accessor.byteOffset + buffer_view.byteOffset;

        if (vertex_memory == nullptr) {
            assert(num_vertices == 0 && vertex_data == nullptr);

            num_vertices = accessor.count;
            vertex_memory = bgfx::alloc(num_vertices * sizeof(Model::BasicModelVertex));
            vertex_data = reinterpret_cast<Model::BasicModelVertex*>(vertex_memory->data);
        }

        if (attribute == "POSITION") {
            const size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(float) * 3 : buffer_view.byteStride;

            if (accessor.type != TINYGLTF_TYPE_VEC3 ||
                accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
                accessor.count != num_vertices ||
                byte_stride < sizeof(float) * 3 ||
                byte_stride * (num_vertices - 1) + sizeof(float) * 3 > buffer_view.byteLength ||
                (attributes & 1) != 0) {
                throw std::runtime_error("Invalid POSITION accessor.");
            }
            attributes |= 1;

            for (size_t i = 0; i < num_vertices; i++) {
                const auto* const position_source_data = reinterpret_cast<const float*>(buffer_data + i * byte_stride);
                vertex_data[i].x = position_source_data[0];
                vertex_data[i].y = position_source_data[1];
                vertex_data[i].z = -position_source_data[2];

                bounds.min_x = std::min(bounds.min_x, vertex_data[i].x);
                bounds.min_y = std::min(bounds.min_y, vertex_data[i].y);
                bounds.min_z = std::min(bounds.min_z, vertex_data[i].z);
                bounds.max_x = std::max(bounds.max_x, vertex_data[i].x);
                bounds.max_y = std::max(bounds.max_y, vertex_data[i].y);
                bounds.max_z = std::max(bounds.max_z, vertex_data[i].z);
            }
        } else if (attribute == "NORMAL") {
            const size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(float) * 3 : buffer_view.byteStride;

            if (accessor.type != TINYGLTF_TYPE_VEC3 ||
                accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
                accessor.count != num_vertices ||
                byte_stride < sizeof(float) * 3 ||
                byte_stride * (num_vertices - 1) + sizeof(float) * 3 > buffer_view.byteLength ||
                (attributes & 2) != 0) {
                throw std::runtime_error("Invalid NORMAL accessor.");
            }
            attributes |= 2;

            for (size_t i = 0; i < num_vertices; i++) {
                const auto* const source_data = reinterpret_cast<const float*>(buffer_data + i * byte_stride);
                vertex_data[i].normal_x = source_data[0];
                vertex_data[i].normal_y = source_data[1];
                vertex_data[i].normal_z = -source_data[2];
            }
        } else if (attribute == "TANGENT") {
            const size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(float) * 4 : buffer_view.byteStride;

            if (accessor.type != TINYGLTF_TYPE_VEC4 ||
                accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
                accessor.count != num_vertices ||
                byte_stride < sizeof(float) * 3 ||
                byte_stride * (num_vertices - 1) + sizeof(float) * 4 > buffer_view.byteLength ||
                (attributes & 4) != 0) {
                throw std::runtime_error("Invalid TANGENT accessor.");
            }
            attributes |= 4;

            for (size_t i = 0; i < num_vertices; i++) {
                const auto* const source_data = reinterpret_cast<const float*>(buffer_data + i * byte_stride);
                vertex_data[i].tangent_x = -source_data[0];
                vertex_data[i].tangent_y = source_data[1];
                vertex_data[i].tangent_z = source_data[2];
                vertex_data[i].tangent_w = source_data[3];
            }
        } else if (attribute == "TEXCOORD_0") {
            if (accessor.type != TINYGLTF_TYPE_VEC2 ||
                accessor.count != num_vertices ||
                (attributes & 8) != 0) {
                throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
            }
            attributes |= 8;

            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                const size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(float) * 2 : buffer_view.byteStride;
                if (byte_stride >= sizeof(float) * 2 && byte_stride * (num_vertices - 1) + sizeof(float) * 2 <= buffer_view.byteLength) {
                    for (size_t i = 0; i < num_vertices; i++) {
                        const auto *const texcoord_source_data = reinterpret_cast<const float *>(buffer_data + i * byte_stride);
                        vertex_data[i].u = texcoord_source_data[0];
                        vertex_data[i].v = texcoord_source_data[1];
                    }
                } else {
                    throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
                }
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                const size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(uint8_t) * 2 : buffer_view.byteStride;
                if (byte_stride >= sizeof(uint8_t) * 2 && byte_stride * (num_vertices - 1) + sizeof(uint8_t) * 2 <= buffer_view.byteLength) {
                    for (size_t i = 0; i < num_vertices; i++) {
                        const auto *const texcoord_source_data = reinterpret_cast<const uint8_t *>(buffer_data + i * byte_stride);
                        vertex_data[i].u = texcoord_source_data[0] / 255.f;
                        vertex_data[i].v = texcoord_source_data[1] / 255.f;
                    }
                } else {
                    throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
                }
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(uint16_t) * 2 : buffer_view.byteStride;
                if (byte_stride >= sizeof(uint16_t) * 2 && byte_stride * (num_vertices - 1) + sizeof(uint16_t) * 2 <= buffer_view.byteLength) {
                    for (size_t i = 0; i < num_vertices; i++) {
                        const auto *const texcoord_source_data = reinterpret_cast<const uint16_t *>(buffer_data + i * byte_stride);
                        vertex_data[i].u = texcoord_source_data[0] / 65535.f;
                        vertex_data[i].v = texcoord_source_data[1] / 65535.f;
                    }
                } else {
                    throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
                }
            } else {
                throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
            }
        }
    }

    if (attributes != 15) {
        std::string missing_attributes;
        if ((attributes & 1) == 0) {
            missing_attributes += "position";
        }
        if ((attributes & 2) == 0) {
            if (!missing_attributes.empty()) {
                missing_attributes += ", ";
            }
            missing_attributes += "normal";
        }
        if ((attributes & 4) == 0) {
            if (!missing_attributes.empty()) {
                missing_attributes += ", ";
            }
            missing_attributes += "tangent";
        }
        if ((attributes & 8) == 0) {
            if (!missing_attributes.empty()) {
                missing_attributes += ", ";
            }
            missing_attributes += "texcoord";
        }
        throw std::runtime_error(fmt::format("Missing attributes: {}", missing_attributes));
    }

    result.vertex_buffer = bgfx::createVertexBuffer(vertex_memory, Model::BasicModelVertex::DECLARATION);
    result.num_vertices = num_vertices;

    if (primitive.indices < 0 || primitive.indices >= model.accessors.size()) {
        throw std::runtime_error("Invalid index accessor.");
    }

    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
    if (accessor.type != TINYGLTF_TYPE_SCALAR ||
        accessor.sparse.isSparse ||
        accessor.count == 0 ||
        accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size()) {
        throw std::runtime_error("Invalid index accessor.");
    }

    const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
    if (buffer_view.target != TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER ||
        buffer_view.buffer < 0 || buffer_view.buffer >= model.buffers.size()) {
        throw std::runtime_error("Invalid index accessor.");
    }

    const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
    if (buffer_view.byteOffset + buffer_view.byteLength > buffer.data.size()) {
        throw std::runtime_error("Invalid index accessor.");
    }

    result.num_indices = accessor.count;

    const uint8_t* const buffer_data = buffer.data.data() + buffer_view.byteOffset;
    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
        if ((buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint8_t)) &&
            buffer_view.byteLength == accessor.count) {
            const bgfx::Memory* memory = bgfx::alloc(accessor.count * sizeof(uint16_t));

            auto* target_data = reinterpret_cast<uint16_t*>(memory->data);
            for (size_t i = 0; i < accessor.count; i++) {
                target_data[i] = uint16_t(buffer_data[i]);
            }

            result.index_buffer = bgfx::createIndexBuffer(memory);
        } else {
            throw std::runtime_error("Invalid BYTE index accessor.");
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        if ((buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint16_t)) &&
            buffer_view.byteLength == accessor.count * sizeof(uint16_t)) {
            result.index_buffer = bgfx::createIndexBuffer(bgfx::copy(buffer_data, buffer_view.byteLength));
        } else {
            throw std::runtime_error("Invalid SHORT index accessor.");
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        const auto* source_data = reinterpret_cast<const uint32_t*>(buffer_data);
        if ((buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint32_t)) &&
            buffer_view.byteLength == accessor.count * sizeof(uint32_t)) {
            const bgfx::Memory* memory = bgfx::alloc(accessor.count * sizeof(uint16_t));

            auto* target_data = reinterpret_cast<uint16_t*>(memory->data);
            for (size_t i = 0; i < accessor.count; i++) {
                target_data[i] = uint16_t(source_data[i]);
            }

            result.index_buffer = bgfx::createIndexBuffer(memory);
        } else {
            throw std::runtime_error("Invalid INT index accessor.");
        }
    } else {
        throw std::runtime_error("Invalid index accessor.");
    }
}

void ResourceSystem::load_presets() const {
    if (auto* preset_single_component = world.try_ctx<PresetSingleComponent>(); preset_single_component != nullptr) {
        std::mutex preset_single_component_mutex;

        const ghc::filesystem::path directory = ghc::filesystem::path(get_resource_directory()) / "presets";
        resource_system_details::iterate_recursive_parallel(directory, ".yaml", [&](const ghc::filesystem::path& file) {
            std::vector<entt::meta_any> preset;
            const std::string name = file.lexically_relative(directory).lexically_normal().string();

            load_preset(preset, file.string());

            std::lock_guard<std::mutex> guard(preset_single_component_mutex);
            preset_single_component->presets.emplace(name, std::move(preset));
        });
    }
}

void ResourceSystem::load_preset(std::vector<entt::meta_any>& result, const std::string &path) const {
    try {
        std::ifstream stream(path);
        if (!stream.is_open()) {
            throw std::runtime_error("Failed to open a file.");
        }

        YAML::Node node = YAML::Load(stream);
        if (!node.IsMap()) {
            throw std::runtime_error("The root node must be a map.");
        }

        for (YAML::const_iterator component_it = node.begin(); component_it != node.end(); ++component_it) {
            if (!component_it->second.IsMap() && !component_it->second.IsNull()) {
                throw std::runtime_error("The component node must be a map.");
            }

            const auto component_name = component_it->first.as<std::string>("");

            entt::meta_type component_type = entt::resolve(entt::hashed_string(component_name.c_str()));
            if (!component_type) {
                throw std::runtime_error(fmt::format("Component type \"{}\" is not registered.", component_name));
            }

            entt::meta_prop ignore_property = component_type.prop("ignore"_hs);
            if (ignore_property && ignore_property.value().can_cast<bool>() && ignore_property.value().cast<bool>()) {
                throw std::runtime_error(fmt::format("Component \"{}\" must not be in a level file.", component_name));
            }

            entt::meta_any component = world.construct_component(component_type);
            if (!component) {
                throw std::runtime_error(fmt::format("Component \"{}\" is not default-constructible.", component_name));
            }

            if (!component_it->second.IsNull()) {
                load_properties(component, component_it->second);
            }
            result.push_back(std::move(component));
        }
    }
    catch (const std::runtime_error& error) {
        throw std::runtime_error(fmt::format("Failed to load model \"{}\".\nDetails: {}", path, error.what()));
    }
}

void ResourceSystem::load_properties(entt::meta_handle object, const YAML::Node& node) const {
    entt::meta_type object_type = object.type();
    for (YAML::const_iterator property_it = node.begin(); property_it != node.end(); ++property_it) {
        const auto property_name = property_it->first.as<std::string>("");

        entt::meta_data property = object_type.data(entt::hashed_string(property_name.c_str()));
        if (!property) {
            throw std::runtime_error(fmt::format("Property \"{}\" not found.", property_name));
        }

        entt::meta_type property_type = property.type();
        if (!property_type) {
            throw std::runtime_error(fmt::format("Property \"{}\" is not registered.", property_name));
        }

        if (property_it->second.IsMap()) {
            if (!property_type.is_class()) {
                throw std::runtime_error(fmt::format("Property \"{}\" is not a structure.", property_name));
            }

            entt::meta_any child_object = property_type.construct();
            if (!child_object) {
                throw std::runtime_error(fmt::format("Property \"{}\" is not default-constructible.", property_name));
            }

            load_properties(child_object, property_it->second);

            if (!property.set(object, child_object)) {
                throw std::runtime_error(fmt::format("Failed to set structure property \"{}\" value.", property_name));
            }
        } else {
            if (!property_it->second.IsScalar()) {
                throw std::runtime_error(fmt::format("Property \"{}\" is neither a map nor scalar.", property_name));
            }

            static entt::meta_type TYPE_INT    = entt::resolve<int32_t>();
            static entt::meta_type TYPE_UINT   = entt::resolve<uint32_t>();
            static entt::meta_type TYPE_FLOAT  = entt::resolve<float>();
            static entt::meta_type TYPE_BOOL   = entt::resolve<bool>();
            static entt::meta_type TYPE_STRING = entt::resolve<std::string>();

            if (property_type == TYPE_INT) {
                if (!property.set(object, property_it->second.as<int32_t>(0))) {
                    throw std::runtime_error(fmt::format("Failed to set integer property \"{}\" value.", property_name));
                }
            } else if (property_type == TYPE_UINT) {
                if (!property.set(object, property_it->second.as<uint32_t>(0))) {
                    throw std::runtime_error(fmt::format("Failed to set unsigned integer property \"{}\" value.", property_name));
                }
            } else if (property_type == TYPE_FLOAT) {
                if (!property.set(object, property_it->second.as<float>(0.f))) {
                    throw std::runtime_error(fmt::format("Failed to set float property \"{}\" value.", property_name));
                }
            } else if (property_type == TYPE_BOOL) {
                if (!property.set(object, property_it->second.as<bool>(false))) {
                    throw std::runtime_error(fmt::format("Failed to set boolean property \"{}\" value.", property_name));
                }
            } else if (property_type == TYPE_STRING) {
                if (!property.set(object, property_it->second.as<std::string>(""))) {
                    throw std::runtime_error(fmt::format("Failed to set string property \"{}\" value.", property_name));
                }
            } else {
                throw std::runtime_error(fmt::format("Property \"{}\" type is not supported.", property_name));
            }
        }
    }
}

void ResourceSystem::load_level() const {
    auto& level_single_component = world.ctx<LevelSingleComponent>();
    auto& guid_single_component = world.set<GuidSingleComponent>();
    auto& name_single_component = world.set<NameSingleComponent>();

    const ghc::filesystem::path level_path = ghc::filesystem::path(get_resource_directory()) / "levels" / level_single_component.level_name;
    if (!ghc::filesystem::exists(level_path)) {
        throw std::runtime_error(fmt::format("Specified level \"{}\" doesn't exist.", level_path.string()));
    }

    try {
        std::ifstream stream(level_path.string());
        if (!stream.is_open()) {
            throw std::runtime_error("Failed to open a file.");
        }

        YAML::Node node = YAML::Load(stream);
        if (!node.IsMap()) {
            throw std::runtime_error("The root node must be a map.");
        }

        YAML::Node entities = node["entities"];
        if (!entities || !entities.IsSequence()) {
            throw std::runtime_error("The entities node must be a sequence.");
        }

        for (YAML::const_iterator entity_it = entities.begin(); entity_it != entities.end(); ++entity_it) {
            if (!entity_it->IsMap()) {
                throw std::runtime_error("The entity node must be a map.");
            }

            entt::entity entity = world.create();

            for (YAML::const_iterator component_it = entity_it->begin(); component_it != entity_it->end(); ++component_it) {
                if (!component_it->second.IsMap() && !component_it->second.IsNull()) {
                    throw std::runtime_error("The component node must be a map.");
                }

                const auto component_name = component_it->first.as<std::string>("");

                entt::meta_type component_type = entt::resolve(entt::hashed_string(component_name.c_str()));
                if (!component_type) {
                    throw std::runtime_error(fmt::format("Component type \"{}\" is not registered.", component_name));
                }

                entt::meta_handle component = world.assign(entity, component_type);
                if (!component_it->second.IsNull()) {
                    load_properties(component, component_it->second);
                }
            }

            if (!world.has<EditorComponent>(entity)) {
                throw std::runtime_error("Entity must have EditorComponent.");
            }

            auto& editor_component = world.get<EditorComponent>(entity);

            if (editor_component.guid > 0x00FFFFFF) {
                throw std::runtime_error(fmt::format("Invalid entity guid {}.", editor_component.guid));
            }
            if (guid_single_component.guid_to_entity.count(editor_component.guid) > 0) {
                throw std::runtime_error(fmt::format("Entity with specified guid {} already exists.", editor_component.guid));
            }
            guid_single_component.guid_to_entity[editor_component.guid] = entity;

            if (editor_component.name.empty()) {
                throw std::runtime_error("Empty entity names are not allowed.");
            }
            if (name_single_component.name_to_entity.count(editor_component.name) > 0) {
                throw std::runtime_error(fmt::format("Entity with specified name \"{}\" already exists.", editor_component.name));
            }
            name_single_component.name_to_entity[editor_component.name] = entity;
        }
    }
    catch (const std::runtime_error& error) {
        throw std::runtime_error(fmt::format("Failed to load level \"{}\".\nDetails: {}", level_path.string(), error.what()));
    }
}

} // namespace hg
