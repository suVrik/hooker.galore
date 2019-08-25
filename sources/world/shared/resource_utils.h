#pragma once

#include <entt/fwd.hpp>

namespace YAML {

class Node;

} // namespace YAML

namespace entt {

class meta_handle;

} // namespace entt

namespace hg {

struct GuidSingleComponent;
struct NameSingleComponent;
class World;

/** `ResourceUtils` is a set of utilities functions for manipulations on resources. */
class ResourceUtils final {
public:
    ResourceUtils() = delete;

    /** Check whether the specified type is registered type. */
    static bool is_registered_type(entt::meta_type type) noexcept;

    /** Serialize the specified POD property into the given YAML node. */
    static void serialize_registered_property(entt::meta_data property, entt::meta_handle object, YAML::Node& node) noexcept;

    /** Deserialize the specified POD property from the given YAML node. */
    static bool deserialize_registered_property(entt::meta_data property, entt::meta_handle object, const YAML::Node& node) noexcept;

    /** Serialize the specified structure property into the given YAML node. */
    static void serialize_structure_property(entt::meta_handle structure, YAML::Node& node) noexcept;

    /** Serialize the specified structure property into the given YAML node. */
    static void deserialize_structure_property(entt::meta_handle structure, const YAML::Node& node) noexcept;

    /** Serialize the specified entity into the given YAML node. */
    static void serialize_entity(World& world, entt::entity entity, YAML::Node& node, bool serialize_editor_component = false) noexcept;

    /** Deserialize the specified entity from the given YAML node. */
    static void deserialize_entity(World& world, entt::entity entity, const YAML::Node& node, GuidSingleComponent* guid_single_component = nullptr, NameSingleComponent* name_single_component = nullptr) noexcept;

    /** Serialize the specified world into the given YAML node. */
    static void serialize_level(World& world, YAML::Node& node, bool serialize_editor_component = false) noexcept;

    /** Serialize the given world into the file specified in `LevelSingleComponent`. */
    static bool serialize_level(World& world, bool serialize_editor_component = false) noexcept;

    /** Deserialize world from the specified YAML node. */
    static void deserialize_level(World& world, const YAML::Node& node, GuidSingleComponent* guid_single_component = nullptr, NameSingleComponent* name_single_component = nullptr) noexcept;

    /** Deserialize world from the file specified in `LevelSingleComponent`. */
    static bool deserialize_level(World& world) noexcept;

    /** Return resource directory path. */
    static std::string get_resource_directory();
};

} // namespace hg
