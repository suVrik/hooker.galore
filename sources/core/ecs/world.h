#pragma once

#include "core/ecs/system.h"

#include <entt/entity/registry.hpp>
#include <entt/meta/factory.hpp>
#include <functional>
#include <string>
#include <vector>

namespace hg {

class System;

/** `World` is an extension over `rtt::registry` that allows to work with components without knowing their compile time
    type and allows to manage and run ECS systems. */
class World final : public entt::registry {
public:
    World();
    ~World();

    /** Register component type `T` so methods expecting `entt::meta_type` and `entt::meta_handle` can be applied to
        components of this type. */
    template <typename T>
    void register_component() noexcept;

    /** Iterate over all registered component types. */
    template <typename T>
    void each_registered_component_type(T callback) const noexcept;

    /** Iterate over all editable component types. */
    template <typename T>
    void each_editable_component_type(T callback) const noexcept;

    /** Construct specified component using default constructor. */
    entt::meta_any construct_component(entt::meta_type component_type) const noexcept;

    /** Acquire copy of specified component using copy constructor. */
    entt::meta_any copy_component(entt::meta_handle component) const noexcept;

    /** Acquire copy of specified component using move constructor. */
    entt::meta_any move_component(entt::meta_handle component) const noexcept;

    /** Return name of specified component. */
    const char* get_component_name(entt::meta_type component_type, const char* fallback = "undefined") const noexcept;

    /** Check whether the specified component is hidden. */
    bool is_component_ignored(entt::meta_type component_type) const noexcept;

    /** Check whether the specified component is editable. Editable component is a non-ignored component with valid
        name, default constructor, copy constructor and copy assignment operator. Only editable components can be
        modified in property editor. Only editable components are copied when entity, they're assigned to, is copied.
        Only editable components are serialized and deserialized from level and preset files. */
    bool is_component_editable(entt::meta_type component_type) const noexcept;

    /** Check whether specified component type is registered. */
    bool is_component_registered(entt::meta_type component_type) const noexcept;

    /** Perform `entt::registry::assign` on earlier registered component. Default constructor is used. */
    entt::meta_handle assign_default(entt::entity entity, entt::meta_type component_type) noexcept;

    /** Perform `entt::registry::assign` on earlier registered component. Copy constructor is used. */
    entt::meta_handle assign_copy(entt::entity entity, entt::meta_handle component) noexcept;

    /** Perform `entt::registry::assign` on earlier registered component. Move constructor is used. */
    entt::meta_handle assign_move(entt::entity entity, entt::meta_handle component) noexcept;

    /** Perform `entt::registry::replace` on earlier registered component. Copy assignment operator is used. */
    entt::meta_handle replace_copy(entt::entity entity, entt::meta_handle component) noexcept;

    /** Perform `entt::registry::replace` on earlier registered component. Move assignment operator is used. */
    entt::meta_handle replace_move(entt::entity entity, entt::meta_handle component) noexcept;

    /** Perform `entt::registry::remove` on earlier registered component. */
    void remove(entt::entity entity, entt::meta_type component_type) noexcept;

    /** Perform `entt::registry::has` on earlier registered component. */
    bool has(entt::entity entity, entt::meta_type component_type) const noexcept;

    /** Perform `entt::registry::try_get` on earlier registered component. */
    entt::meta_handle get(entt::entity entity, entt::meta_type component_type) const noexcept;

    /** Perform `entt::registry::get_or_assign` on earlier registered component. Default constructor is used. */
    entt::meta_handle get_or_assign(entt::entity entity, entt::meta_type component_type) noexcept;

    /** Check whether component type is default constructible (required by `assign_default` and `get_or_assign` methods). */
    bool is_default_constructible(entt::meta_type component_type) const noexcept;

    /** Check whether component type is copy constructible (required by `assign_copy` method). */
    bool is_copy_constructible(entt::meta_type component_type) const noexcept;

    /** Check whether component type is move constructible (required by `assign_move` method). */
    bool is_move_constructible(entt::meta_type component_type) const noexcept;

    /** Check whether component type is copy assignable (required by `replace_copy` method). */
    bool is_copy_assignable(entt::meta_type component_type) const noexcept;

    /** Check whether component type is copy assignable (required by `replace_move` method). */
    bool is_move_assignable(entt::meta_type component_type) const noexcept;

    /** Iterate over all registered components of specified `entity`. */
    template <typename T>
    void each_registered_entity_component(entt::entity entity, T callback) const noexcept;

    /** Iterate over all editable components of specified `entity`. */
    template <typename T>
    void each_editable_entity_component(entt::entity entity, T callback) const noexcept;

    /** Allow using entt versions of `assign`, `remove`, `has`, `get` and `get_or_assign` methods. */
    using entt::registry::remove;
    using entt::registry::has;
    using entt::registry::get;
    using entt::registry::get_or_assign;

    /** Register system `T` with member function `update` and specified `name`. */
    template <typename T>
    void register_system(const std::string& name) noexcept;

    /** Specify in which order systems of specified type should be executed. */
    void normal_system_order(const std::vector<std::string>& system_order) noexcept;
    void fixed_system_order(const std::vector<std::string>& system_order) noexcept;

    /** Construct all the systems specified by `normal_system_order` and `fixed_system_order` methods. */
    void construct_systems();

    /** Execute all systems of specified type. */
    bool update_normal(float elapsed_time) noexcept;
    void update_fixed(float elapsed_time) noexcept;

private:
    struct ComponentDescriptor final {
        entt::meta_any(*construct)();
        entt::meta_any(*copy)(entt::meta_handle component);
        entt::meta_any(*move)(entt::meta_handle component);
        entt::meta_handle(*assign_default)(World* world, entt::entity entity);
        entt::meta_handle(*assign_copy)(World* world, entt::entity entity, entt::meta_handle component);
        entt::meta_handle(*assign_move)(World* world, entt::entity entity, entt::meta_handle component);
        entt::meta_handle(*replace_copy)(World* world, entt::entity entity, entt::meta_handle component);
        entt::meta_handle(*replace_move)(World* world, entt::entity entity, entt::meta_handle component);
        void(*remove)(World* world, entt::entity entity);
        bool(*has)(const World* world, entt::entity entity);
        entt::meta_handle(*get)(const World* world, entt::entity entity);
        entt::meta_handle(*get_or_assign)(World* world, entt::entity entity);
    };

    std::unordered_map<entt::meta_type, ComponentDescriptor> m_components;

    struct SystemDescriptor final {
        std::function<std::unique_ptr<System>(World& world)> construct;
        std::unique_ptr<System> system;
    };

    std::unordered_map<std::string, SystemDescriptor> m_systems[2];
    std::vector<std::string> m_system_order[2];
};

/** Presence of this single component means the world may keep running. */
struct RunningWorldSingleComponent final {
    bool dummy = false;
};

} // namespace hg

#include "core/ecs/world_impl.h"
