#include "core/ecs/world.h"
#include "world/editor/editor_component.h"
#include "world/editor/guid_single_component.h"
#include "world/shared/level_single_component.h"
#include "world/shared/name_single_component.h"
#include "world/shared/resource_utils.h"

#include <SDL2/SDL_filesystem.h>
#include <entt/meta/factory.hpp>
#include <fmt/format.h>
#include <ghc/filesystem.hpp>
#include <iostream>
#include <yaml-cpp/yaml.h>

#define RESOURCE_WARNING assert(false); std::cout << "[RESOURCE] "

namespace hg {

namespace resource_utils_details {

static entt::meta_type TYPE_INT    = entt::resolve<int32_t>();
static entt::meta_type TYPE_UINT   = entt::resolve<uint32_t>();
static entt::meta_type TYPE_FLOAT  = entt::resolve<float>();
static entt::meta_type TYPE_BOOL   = entt::resolve<bool>();
static entt::meta_type TYPE_STRING = entt::resolve<std::string>();

const std::vector<entt::meta_type> REGISTERED_TYPES = {
        TYPE_INT,
        TYPE_UINT,
        TYPE_FLOAT,
        TYPE_BOOL,
        TYPE_STRING,
};

} // namespace resource_utils_details

bool ResourceUtils::is_registered_type(const entt::meta_type type) noexcept {
    using namespace resource_utils_details;
    return std::find(REGISTERED_TYPES.begin(), REGISTERED_TYPES.end(), type) != REGISTERED_TYPES.end();
}

void ResourceUtils::serialize_registered_property(const entt::meta_data property, const entt::meta_handle object, YAML::Node& node) noexcept {
    using namespace resource_utils_details;

    assert(property);
    assert(object);
    assert(node.IsScalar());

    const entt::meta_type property_type = property.type();
    assert(is_registered_type(property_type));

    const entt::meta_any property_value = property.get(object);
    assert(property_value);

    if (property_type == TYPE_INT) {
        node = property_value.cast<int32_t>();
    } else if (property_type == TYPE_UINT) {
        node = property_value.cast<uint32_t>();
    } else if (property_type == TYPE_FLOAT) {
        node = property_value.cast<float>();
    } else if (property_type == TYPE_BOOL) {
        node = property_value.cast<bool>();
    } else if (property_type == TYPE_STRING) {
        node = property_value.cast<std::string>();
    } else {
        assert(false);
        node = 0;
    }
}

bool ResourceUtils::deserialize_registered_property(const entt::meta_data property, const entt::meta_handle object, const YAML::Node& node) noexcept {
    using namespace resource_utils_details;

    assert(property);
    assert(object);
    assert(node.IsScalar());
    
    const entt::meta_type property_type = property.type();
    assert(is_registered_type(property_type));

    if (property_type == TYPE_INT) {
        return property.set(object, node.as<int32_t>(0));
    } else if (property_type == TYPE_UINT) {
        return property.set(object, node.as<uint32_t>(0));
    } else if (property_type == TYPE_FLOAT) {
        return property.set(object, node.as<float>(0.f));
    } else if (property_type == TYPE_BOOL) {
        return property.set(object, node.as<bool>(false));
    } else if (property_type == TYPE_STRING) {
        return property.set(object, node.as<std::string>(""));
    } else {
        assert(false);
        return false;
    }
}

void ResourceUtils::serialize_structure_property(const entt::meta_handle structure, YAML::Node& node) noexcept {
    assert(structure);
    assert(node.IsMap());

    const entt::meta_type structure_type = structure.type();
    assert(structure_type);
    assert(structure_type.is_class());

    structure_type.data([&](const entt::meta_data property) {
        const entt::meta_type property_type = property.type();
        if (property_type) {
            const entt::meta_prop ignore_property = property_type.prop("ignore"_hs);
            if (!ignore_property || !ignore_property.value().can_cast<bool>() || !ignore_property.value().cast<bool>()) {
                const entt::meta_prop name_property = property.prop("name"_hs);
                if (name_property && name_property.value().can_cast<const char*>()) {
                    const std::string name = name_property.value().cast<const char*>();
                    assert(!name.empty());

                    if (is_registered_type(property_type)) {
                        YAML::Node& child_node = node[name] = YAML::Node(YAML::NodeType::Scalar);
                        serialize_registered_property(property, structure, child_node);
                    } else if (property_type.is_class()) {
                        entt::meta_any value = property.get(structure);
                        assert(value);

                        YAML::Node& child_node = node[name] = YAML::Node(YAML::NodeType::Map);
                        serialize_structure_property(value, child_node);
                    }
                }
            }
        }
    });
}

void ResourceUtils::deserialize_structure_property(const entt::meta_handle structure, const YAML::Node& node) noexcept {
    assert(structure);
    assert(node.IsMap());

    const entt::meta_type structure_type = structure.type();
    assert(structure_type);

    for (YAML::const_iterator property_it = node.begin(); property_it != node.end(); ++property_it) {
        const auto property_name = property_it->first.as<std::string>("");
        assert(!property_name.empty());

        const entt::meta_data property = structure_type.data(entt::hashed_string(property_name.c_str()));
        if (property) {
            const entt::meta_prop ignore_property = property.prop("ignore"_hs);
            if (!ignore_property || !ignore_property.value().can_cast<bool>() || !ignore_property.value().cast<bool>()) {
                const entt::meta_type property_type = property.type();
                if (property_type) {
                    if (is_registered_type(property_type)) {
                        if (!deserialize_registered_property(property, structure, property_it->second)) {
                            RESOURCE_WARNING << "Failed to deserialize property \"" << property_name << "\"." << std::endl;
                        }
                    } else {
                        if (property_type.is_class() && property_it->second.IsMap()) {
                            entt::meta_any child_structure = property_type.construct();
                            if (child_structure) {
                                deserialize_structure_property(child_structure, property_it->second);

                                if (!property.set(structure, child_structure)) {
                                    RESOURCE_WARNING << "Failed to set structure property's \"" << property_name << "\"  value." << std::endl;
                                }
                            } else {
                                RESOURCE_WARNING << "Structure property's \"" << property_name << "\" is not default-constructible." << std::endl;
                            }
                        } else {
                            RESOURCE_WARNING << "Property \"" << property_name << "\" type mismatch." << std::endl;
                        }
                    }
                } else {
                    RESOURCE_WARNING << "Unknown property's \"" << property_name << "\" type." << std::endl;
                }
            } else {
                RESOURCE_WARNING << "Ignored property \"" << property_name << "\" is specified." << std::endl;
            }
        } else {
            RESOURCE_WARNING << "Unknown property \"" << property_name << "\" is specified." << std::endl;
        }
    }
}

void ResourceUtils::serialize_entity(World& world, const entt::entity entity, YAML::Node& node, const bool serialize_editor_component) noexcept {
    assert(world.valid(entity));
    assert(node.IsMap());

    world.each(entity, [&](const entt::meta_handle component_handle) {
        assert(component_handle);

        const entt::meta_type component_type = component_handle.type();
        assert(component_type);

        if (!serialize_editor_component && component_type == entt::resolve<EditorComponent>()) {
            return;
        }

        const entt::meta_prop ignore_property = component_type.prop("ignore"_hs);
        if (!ignore_property || !ignore_property.value().can_cast<bool>() || !ignore_property.value().cast<bool>()) {
            const entt::meta_prop name_property = component_type.prop("name"_hs);
            if (name_property && name_property.value().can_cast<const char*>()) {
                const std::string component_name = name_property.value().cast<const char*>();
                YAML::Node& component_node = node[component_name] = YAML::Node(YAML::NodeType::Map);
                serialize_structure_property(component_handle, component_node);
            }
        }
    });
}

void ResourceUtils::deserialize_entity(World& world, const entt::entity entity, const YAML::Node& node, GuidSingleComponent* const guid_single_component, NameSingleComponent* const name_single_component) noexcept {
    assert(world.valid(entity));
    assert(node.IsMap());
    assert((guid_single_component == nullptr) == (name_single_component == nullptr));

    for (YAML::const_iterator component_it = node.begin(); component_it != node.end(); ++component_it) {
        const auto component_name = component_it->first.as<std::string>("");
        assert(!component_name.empty());

        if (component_it->second.IsMap()) {
            const entt::meta_type component_type = entt::resolve(entt::hashed_string(component_name.c_str()));
            if (component_type) {
                const entt::meta_prop ignore_property = component_type.prop("ignore"_hs);
                if (!ignore_property || !ignore_property.value().can_cast<bool>() || !ignore_property.value().cast<bool>()) {
                    if (world.is_component_registered(component_type)) {
                        if (!world.has(entity, component_type)) {
                            const bool is_editor_component = component_type == entt::resolve<EditorComponent>();
                            const bool allow_editor_component = guid_single_component != nullptr && name_single_component != nullptr;
                            if (!allow_editor_component && is_editor_component) {
                                continue;
                            }

                            entt::meta_any component = world.construct_component(component_type);
                            if (component) {
                                deserialize_structure_property(component, component_it->second);

                                if (is_editor_component) {
                                    auto& editor_component = component.cast<EditorComponent>();

                                    if (guid_single_component->guid_to_entity.count(editor_component.guid) > 0) {
                                        RESOURCE_WARNING << "Entity with guid \"" << editor_component.guid << "\" already exists." << std::endl;
                                        editor_component.guid = guid_single_component->acquire_unique_guid(entity);
                                    } else {
                                        guid_single_component->guid_to_entity[editor_component.guid] = entity;
                                    }

                                    if (name_single_component->name_to_entity.count(editor_component.name) > 0) {
                                        RESOURCE_WARNING << "Entity with name \"" << editor_component.guid << "\" already exists." << std::endl;
                                        editor_component.name = name_single_component->acquire_unique_name(entity, editor_component.name);
                                    } else {
                                        name_single_component->name_to_entity[editor_component.name] = entity;
                                    }
                                }

                                world.assign(entity, component);
                            } else {
                                RESOURCE_WARNING << "Failed to construct component \"" << component_name << "\"." << std::endl;
                            }
                        } else {
                            RESOURCE_WARNING << "Component \"" << component_name << "\" is already assigned." << std::endl;
                        }
                    } else {
                        RESOURCE_WARNING << "Component \"" << component_name << "\" is not registered." << std::endl;
                    }
                } else {
                    RESOURCE_WARNING << "Ignored component \"" << component_name << "\" is specified." << std::endl;
                }
            } else {
                RESOURCE_WARNING << "Unknown component \"" << component_name << "\" is specified." << std::endl;
            }
        } else {
            RESOURCE_WARNING << "Corrupted component \"" << component_name << "\" is specified." << std::endl;
        }
    }
}

void ResourceUtils::serialize_level(World& world, YAML::Node& node, const bool serialize_editor_component) noexcept {
    assert(node.IsSequence());

    entt::view<EditorComponent> entities = world.view<EditorComponent>();
    for (entt::entity entity : entities) {
        YAML::Node child_node(YAML::NodeType::Map);
        serialize_entity(world, entity, child_node, serialize_editor_component);
        if (child_node.size() > 0) {
            node.push_back(child_node);
        }
    }
}

bool ResourceUtils::serialize_level(World& world, const bool serialize_editor_component) noexcept {
    auto& level_single_component = world.ctx<LevelSingleComponent>();
    assert(!level_single_component.level_name.empty());

    YAML::Node root_node(YAML::NodeType::Map);
    YAML::Node& entities_node = root_node["entities"] = YAML::Node(YAML::NodeType::Sequence);
    serialize_level(world, entities_node, serialize_editor_component);

    const ghc::filesystem::path level_path = ghc::filesystem::path(get_resource_directory()) / "levels" / level_single_component.level_name;

    std::ofstream stream(level_path.string());
    if (!stream.is_open()) {
        RESOURCE_WARNING << "Failed to open \"" << level_path.string() << "\"." << std::endl;
        return false;
    }

    if (!(stream << root_node)) {
        RESOURCE_WARNING << "Failed to write to \"" << level_path.string() << "\"." << std::endl;
        return false;
    }

    return true;
}

void ResourceUtils::deserialize_level(World& world, const YAML::Node& node, GuidSingleComponent* const guid_single_component, NameSingleComponent* const name_single_component) noexcept {
    assert(node.IsSequence());
    assert((guid_single_component == nullptr) == (name_single_component == nullptr));

    for (YAML::const_iterator entity_it = node.begin(); entity_it != node.end(); ++entity_it) {
        if (entity_it->IsMap()) {
            const entt::entity entity = world.create();
            deserialize_entity(world, entity, *entity_it, guid_single_component, name_single_component);
        } else {
            RESOURCE_WARNING << "Corrupted entity is specified." << std::endl;
        }
    }
}

bool ResourceUtils::deserialize_level(World& world) noexcept {
    auto& level_single_component = world.ctx<LevelSingleComponent>();
    auto& guid_single_component = world.set<GuidSingleComponent>();
    auto& name_single_component = world.set<NameSingleComponent>();

    const ghc::filesystem::path level_path = ghc::filesystem::path(get_resource_directory()) / "levels" / level_single_component.level_name;
    if (!ghc::filesystem::exists(level_path)) {
        RESOURCE_WARNING << "Specified level \"" << level_path.string()  << "\" doesn't exist." << std::endl;
        return false;
    }

    std::ifstream stream(level_path.string());
    if (!stream.is_open()) {
        RESOURCE_WARNING << "Failed to open \"" << level_path.string() << "\"." << std::endl;
        return false;
    }

    const YAML::Node node = YAML::Load(stream);
    if (!node.IsMap()) {
        RESOURCE_WARNING << "The root node of the level must be a map." << std::endl;
        return false;
    }

    const YAML::Node entities = node["entities"];
    if (!entities.IsSequence()) {
        RESOURCE_WARNING << "The entities node must be a sequence." << std::endl;
        return false;
    }

    deserialize_level(world, entities, &guid_single_component, &name_single_component);

    return true;
}

std::string ResourceUtils::get_resource_directory() {
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

} // namespace hg

#undef RESOURCE_WARNING
