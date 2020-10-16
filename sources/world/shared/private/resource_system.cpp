#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "core/resource/texture.h"
#include "world/editor/editor_preset_single_component.h"
#include "world/render/material_component.h"
#include "world/render/render_tags.h"
#include "world/render/texture_single_component.h"
#include "world/shared/resource_system.h"
#include "world/shared/resource_utils.h"

#include <algorithm>
#include <bgfx/bgfx.h>
#include <bx/debug.h>
#include <bx/file.h>
#include <entt/meta/factory.hpp>
#include <fmt/format.h>
#include <future>
#include <ghc/filesystem.hpp>
#include <iostream>
#include <mutex>
#include <yaml-cpp/yaml.h>

namespace hg {

namespace resource_system_details {

bool compare_case_insensitive(const std::string& a, const std::string& b) {
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

static const uint8_t RED_TEXTURE[4] = { 0xFF, 0x00, 0x00, 0xFF };

} // namespace resource_system_details

SYSTEM_DESCRIPTOR(
    SYSTEM(ResourceSystem),
     // TODO: Split ResourceSystem to several systems. RenderResourceSystem may load textures and meshes.
    //        EditorResourceSystem may load presets. ResourceSystem may load physical meshes and level.
    TAGS(render),
    BEFORE("RenderSystem"),
    AFTER("RenderFetchSystem")
)

ResourceSystem::ResourceSystem(World& world)
        : NormalSystem(world)
        , m_material_observer(entt::observer(world, entt::collector.group<MaterialComponent>()))
        , m_material_update_observer(entt::observer(world, entt::collector.replace<MaterialComponent>())) {
    load_textures();

    try {
        load_presets();
        if (!ResourceUtils::deserialize_level(world)) {
            throw std::runtime_error("Failed to load a level.");
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
}

void ResourceSystem::update(float /*elapsed_time*/) {
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

Texture ResourceSystem::load_texture(const std::string &path) const {
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
                        if (ComponentManager::is_registered(component_type)) {
                            if (ComponentManager::is_editable(component_type)) {
                                entt::meta_any component = ComponentManager::construct(component_type);
                                assert(component && "Failed to construct editable component.");

                                ResourceUtils::deserialize_structure_property(component, component_it->second);
                                result.push_back(std::move(component));
                            } else {
                                bx::debugPrintf("[RESOURCE] Preset component \"%s\" is not editable.\n", component_name.c_str());
                            }
                        } else {
                            bx::debugPrintf("[RESOURCE] Preset component \"%s\" is not registered.\n", component_name.c_str());
                        }
                    } else {
                        bx::debugPrintf("[RESOURCE] Unknown preset component \"%s\" is specified.\n", component_name.c_str());
                    }
                } else {
                    bx::debugPrintf("[RESOURCE] Corrupted preset component \"%s\" is specified.\n", component_name.c_str());
                }
            }
        } else {
            bx::debugPrintf("[RESOURCE] Corrupted preset \"%s\" is specified.\n", path.c_str());
        }
    } else {
        bx::debugPrintf("[RESOURCE] Failed to open preset \"%s\".\n", path.c_str());
    }
}

} // namespace hg
