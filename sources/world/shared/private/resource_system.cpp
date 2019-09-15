#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/resource/texture.h"
#include "world/editor/editor_preset_single_component.h"
#include "world/shared/render/material_component.h"
#include "world/shared/render/model_component.h"
#include "world/shared/render/model_single_component.h"
#include "world/shared/render/texture_single_component.h"
#include "world/shared/resource_system.h"
#include "world/shared/resource_utils.h"

#define TINYGLTF_NO_STB_IMAGE       1
#define TINYGLTF_NO_STB_IMAGE_WRITE 1

#include <algorithm>
#include <bgfx/bgfx.h>
#include <bx/file.h>
#include <entt/meta/factory.hpp>
#include <fmt/format.h>
#include <future>
#include <ghc/filesystem.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <mutex>
#include <tiny_gltf.h>
#include <yaml-cpp/yaml.h>

#define RESOURCE_WARNING assert(false); std::cout << "[RESOURCE] "

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
        destroy_model_node(child_node);
    }
    delete node.mesh;
}

static const uint8_t RED_TEXTURE[4] = { 0xFF, 0x00, 0x00, 0xFF };

} // namespace resource_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(ResourceSystem),
     // TODO: Split ResourceSystem to several systems. RenderResourceSystem may load textures and meshes.
    //        EditorResourceSystem may load presets. ResourceSystem may load physical meshes and level.
    REQUIRE("render"),
    BEFORE("RenderSystem"),
    AFTER("RenderFetchSystem")
)

ResourceSystem::ResourceSystem(World& world)
        : NormalSystem(world)
        , m_model_observer(entt::observer(world, entt::collector.group<ModelComponent>()))
        , m_model_update_observer(entt::observer(world, entt::collector.replace<ModelComponent>()))
        , m_material_observer(entt::observer(world, entt::collector.group<MaterialComponent>()))
        , m_material_update_observer(entt::observer(world, entt::collector.replace<MaterialComponent>())) {
    load_textures();

    try {
        load_models();

        try {
            load_presets();
            if (!ResourceUtils::deserialize_level(world)) {
                throw std::runtime_error("Failed to load a level.");
            }
        }
        catch (...) {
            auto& model_single_component = world.ctx<ModelSingleComponent>();
            for (auto& [material_name, material_ptr] : std::exchange(model_single_component.m_models, {})) {
                for (Model::Node& node : material_ptr->children) {
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
    texture_single_component.m_default_texture.~Texture();
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

    auto& texture_single_component = world.ctx<TextureSingleComponent>();

    auto material_updated = [&](const entt::entity entity) {
        auto& material_component = world.get<MaterialComponent>(entity);
        if (!material_component.material.empty()) {
            material_component.color_roughness = &texture_single_component.get(material_component.material + "_bcr.dds");
            material_component.normal_metal_ao = &texture_single_component.get(material_component.material + "_nmao.dds");
        } else {
            material_component.color_roughness = nullptr;
            material_component.normal_metal_ao = nullptr;
        }
    };

    m_material_observer.each(material_updated);
    m_material_update_observer.each(material_updated);
}

void ResourceSystem::load_textures() const {
    auto& texture_single_component = world.set<TextureSingleComponent>();
    std::mutex texture_single_component_mutex;

    const ghc::filesystem::path directory = ghc::filesystem::path(ResourceUtils::get_resource_directory()) / "textures";
    resource_system_details::iterate_recursive_parallel(directory, ".dds", [&](const ghc::filesystem::path& file) {
        Texture texture = load_texture(file.string());
        if (bgfx::isValid(texture.handle)) {
            std::lock_guard<std::mutex> guard(texture_single_component_mutex);
            const std::string texture_name = file.lexically_relative(directory).lexically_normal().string();
            bgfx::setName(texture.handle, texture_name.c_str());
            texture_single_component.m_textures.emplace(texture_name, std::move(texture));
        } else {
            std::lock_guard<std::mutex> guard(resource_system_details::output_mutex);
            std::cerr << "[RESOURCE] Failed to load texture \"" << file.string() << "\"." << std::endl;
        }
    });

    // Red square texture used as a fallback texture.
    const bgfx::Memory* memory = bgfx::makeRef(resource_system_details::RED_TEXTURE, static_cast<uint32_t>(std::size(resource_system_details::RED_TEXTURE)));
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
            }
        }
        bx::close(&file_reader);
    }
    return result;
}

void ResourceSystem::load_models() const  {
    auto& model_single_component = world.set<ModelSingleComponent>();
    std::mutex model_single_component_mutex;

    const ghc::filesystem::path directory = ghc::filesystem::path(ResourceUtils::get_resource_directory()) / "models";
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
                    if (node_index >= model.nodes.size()) {
                        throw std::runtime_error("Invalid node index.");
                    }

                    const glm::mat4 transform(1.f);
                    load_model_node(transform, result.children.emplace_back(), result.bounds, model, model.nodes[node_index]);
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

void ResourceSystem::load_model_node(const glm::mat4& parent_transform, Model::Node& result, Model::AABB& bounds, const tinygltf::Model &model, const tinygltf::Node &node) const {
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
            result.translation = glm::vec3(static_cast<float>(node.translation[0]), static_cast<float>(node.translation[1]), static_cast<float>(-node.translation[2]));
        } else {
            result.translation = glm::vec3(0.f);
        }

        if (node.rotation.size() == 4) {
            result.rotation = glm::quat(static_cast<float>(node.rotation[3]), static_cast<float>(-node.rotation[0]), static_cast<float>(-node.rotation[1]), static_cast<float>(node.rotation[2]));
        } else {
            result.rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        }

        if (node.scale.size() == 3) {
            result.scale = glm::vec3(static_cast<float>(node.scale[0]), static_cast<float>(node.scale[1]), static_cast<float>(node.scale[2]));
        } else {
            result.scale = glm::vec3(1.f, 1.f, 1.f);
        }
    }

    glm::mat4 local_transform = glm::translate(glm::mat4(1.f), result.translation);
    local_transform = local_transform * glm::mat4_cast(result.rotation);
    local_transform = glm::scale(local_transform, result.scale);
    const glm::mat4 transform = parent_transform * local_transform;

    if (node.mesh > -1 && node.mesh < model.meshes.size()) {
        result.mesh = new Model::Mesh();
        load_model_mesh(transform, *result.mesh, bounds, model, node);
    }

    for (const int child_index : node.children) {
        if (child_index >= 0 && child_index < model.nodes.size()) {
            load_model_node(transform, result.children.emplace_back(), bounds, model, model.nodes[child_index]);
        } else {
            throw std::runtime_error("Invalid child.");
        }
    }
}

void ResourceSystem::load_model_mesh(const glm::mat4& parent_transform, Model::Mesh& result, Model::AABB& bounds, const tinygltf::Model &model, const tinygltf::Node &node) const {
    const tinygltf::Mesh& mesh = model.meshes[node.mesh];
    for (const tinygltf::Primitive& primitive : mesh.primitives) {
        if (primitive.mode == TINYGLTF_MODE_TRIANGLES) {
            load_model_primitive(parent_transform, result.primitives.emplace_back(), bounds, model, primitive);
        }
    }
}

void ResourceSystem::load_model_primitive(const glm::mat4& parent_transform, Model::Primitive& result, Model::AABB& bounds, const tinygltf::Model &model, const tinygltf::Primitive& primitive) const {
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
            vertex_memory = bgfx::alloc(static_cast<uint32_t>(num_vertices * sizeof(Model::BasicModelVertex)));
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

                glm::vec4 transformed_vertex(parent_transform * glm::vec4(vertex_data[i].x, vertex_data[i].y, vertex_data[i].z, 1.f));

                bounds.min_x = std::min(bounds.min_x, transformed_vertex.x);
                bounds.min_y = std::min(bounds.min_y, transformed_vertex.y);
                bounds.min_z = std::min(bounds.min_z, transformed_vertex.z);
                bounds.max_x = std::max(bounds.max_x, transformed_vertex.x);
                bounds.max_y = std::max(bounds.max_y, transformed_vertex.y);
                bounds.max_z = std::max(bounds.max_z, transformed_vertex.z);
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
            const bgfx::Memory* memory = bgfx::alloc(static_cast<uint32_t>(accessor.count * sizeof(uint16_t)));

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
            result.index_buffer = bgfx::createIndexBuffer(bgfx::copy(buffer_data, static_cast<uint32_t>(buffer_view.byteLength)));
        } else {
            throw std::runtime_error("Invalid SHORT index accessor.");
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        const auto* source_data = reinterpret_cast<const uint32_t*>(buffer_data);
        if ((buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint32_t)) &&
            buffer_view.byteLength == accessor.count * sizeof(uint32_t)) {
            const bgfx::Memory* memory = bgfx::alloc(static_cast<uint32_t>(accessor.count * sizeof(uint16_t)));

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
    if (auto* editor_preset_single_component = world.try_ctx<EditorPresetSingleComponent>(); editor_preset_single_component != nullptr) {
        std::mutex preset_single_component_mutex;

        const ghc::filesystem::path directory = ghc::filesystem::path(ResourceUtils::get_resource_directory()) / "presets";
        resource_system_details::iterate_recursive_parallel(directory, ".yaml", [&](const ghc::filesystem::path& file) {
            std::vector<entt::meta_any> preset;
            const std::string name = file.lexically_relative(directory).lexically_normal().string();

            load_preset(preset, file.string());

            std::lock_guard<std::mutex> guard(preset_single_component_mutex);
            editor_preset_single_component->presets.emplace(name, std::move(preset));
        });
    }
}

void ResourceSystem::load_preset(std::vector<entt::meta_any>& result, const std::string &path) const {
    std::ifstream stream(path);
    if (stream.is_open()) {
        YAML::Node node = YAML::Load(stream);
        if (node.IsMap()) {
            for (YAML::const_iterator component_it = node.begin(); component_it != node.end(); ++component_it) {
                const auto component_name = component_it->first.as<std::string>("");
                assert(!component_name.empty());

                if (component_it->second.IsMap()) {
                    const entt::meta_type component_type = entt::resolve(entt::hashed_string(component_name.c_str()));
                    if (component_type) {
                        if (world.is_component_registered(component_type)) {
                            if (world.is_component_editable(component_type)) {
                                entt::meta_any component = world.construct_component(component_type);
                                assert(component && "Failed to construct editable component.");

                                ResourceUtils::deserialize_structure_property(component, component_it->second);
                                result.push_back(std::move(component));
                            } else {
                                RESOURCE_WARNING << "Preset component \"" << component_name << "\" is not editable." << std::endl;
                            }
                        } else {
                            RESOURCE_WARNING << "Preset component \"" << component_name << "\" is not registered." << std::endl;
                        }
                    } else {
                        RESOURCE_WARNING << "Unknown preset component \"" << component_name << "\" is specified." << std::endl;
                    }
                } else {
                    RESOURCE_WARNING << "Corrupted preset component \"" << component_name << "\" is specified." << std::endl;
                }
            }
        } else {
            RESOURCE_WARNING << "Corrupted preset \"" << path << "\" is specified." << std::endl;
        }
    } else {
        RESOURCE_WARNING << "Failed to open preset \"" << path << "\"." << std::endl;
    }
}

} // namespace hg

#undef RESOURCE_WARNING
