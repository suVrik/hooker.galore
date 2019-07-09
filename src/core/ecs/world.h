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

    /** Construct specified component using default constructor. */
    entt::meta_any construct_component(entt::meta_type component) const noexcept;

    /** Acquire copy of specified component using copy constructor. */
    entt::meta_any copy_component(const entt::meta_handle& component) const noexcept;

    /** Return name of specified component. */
    const char* get_component_name(entt::meta_type component) const noexcept;

    /** Perform `entt::registry::assign` on earlier registered component. Default constructor is used. */
    entt::meta_handle assign(entt::entity entity, entt::meta_type component) noexcept;

    /** Perform `entt::registry::assign` on earlier registered component. Copy constructor is used. */
    entt::meta_handle assign(entt::entity entity, const entt::meta_handle& component) noexcept;

    /** Perform `entt::registry::replace` on earlier registered component. Copy constructor is used. */
    entt::meta_handle replace(entt::entity entity, const entt::meta_handle& component) noexcept;

    /** Perform `entt::registry::remove` on earlier registered component. */
    void remove(entt::entity entity, entt::meta_type component_type) noexcept;

    /** Perform `entt::registry::has` on earlier registered component. */
    bool has(entt::entity entity, entt::meta_type component) const noexcept;

    /** Perform `entt::registry::try_get` on earlier registered component. */
    entt::meta_handle get(entt::entity entity, entt::meta_type component) const noexcept;

    /** Perform `entt::registry::get_or_assign` on earlier registered component. Default constructor is used. */
    entt::meta_handle get_or_assign(entt::entity entity, entt::meta_type component) noexcept;

    /** Iterate over all registered components of specified `entity`. */
    template <typename T>
    void each(entt::entity entity, T callback) const noexcept;

    /** Allow using entt versions of `assign`, `remove`, `has`, `get` and `get_or_assign` methods. */
    using entt::registry::assign;
    using entt::registry::remove;
    using entt::registry::has;
    using entt::registry::get;
    using entt::registry::get_or_assign;
    using entt::registry::each;
    using entt::registry::replace;

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
        entt::meta_any(*copy)(const entt::meta_handle& component);
        entt::meta_handle(*assign_default)(World* world, entt::entity entity);
        entt::meta_handle(*assign_copy)(World* world, entt::entity entity, const entt::meta_handle& component);
        entt::meta_handle(*replace)(World* world, entt::entity entity, const entt::meta_handle& component);
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

/** Presence of this single component in world entity means the world may keep running. */
struct RunningWorldSingleComponent final {
    bool dummy = false;
};

} // namespace hg

#include "core/ecs/world_impl.h"
