#pragma once

#include "core/ecs/component_manager.h"
#include "core/ecs/system.h"

#include <entt/entity/registry.hpp>
#include <entt/meta/factory.hpp>
#include <string>
#include <vector>

namespace hg {

/** `World` is an extension over `entt::registry` that allows to work with components without knowing their compile time
    type and allows to manage and run ECS systems. */
class World final : public entt::registry {
public:
    World();
    ~World();

    /** Perform `entt::registry::assign` on earlier registered component. Component must be default-constructible. */
    entt::meta_handle assign_default(entt::entity entity, entt::meta_type component_type);

    /** Perform `entt::registry::assign` on earlier registered component. Component must be copy-constructible. */
    entt::meta_handle assign_copy(entt::entity entity, entt::meta_handle component);

    /** Perform `entt::registry::assign` on earlier registered component. Component must be move-constructible. */
    entt::meta_handle assign_move(entt::entity entity, entt::meta_handle component);

    /** Perform `entt::registry::assign` on earlier registered component. Component must be move-constructible or copy-constructible. */
    entt::meta_handle assign_move_or_copy(entt::entity entity, entt::meta_handle component);

    /** Perform `entt::registry::replace` on earlier registered component. Component must be copy-assignable. */
    entt::meta_handle replace_copy(entt::entity entity, entt::meta_handle component);

    /** Perform `entt::registry::replace` on earlier registered component. Component must be move-assignable. */
    entt::meta_handle replace_move(entt::entity entity, entt::meta_handle component);

    /** Perform `entt::registry::replace` on earlier registered component. Component must be move-assignable or copy-assignable. */
    entt::meta_handle replace_move_or_copy(entt::entity entity, entt::meta_handle component);

    /** Perform `entt::registry::remove` on earlier registered component. */
    void remove(entt::entity entity, entt::meta_type component_type);

    /** Perform `entt::registry::has` on earlier registered component. */
    bool has(entt::entity entity, entt::meta_type component_type) const;

    /** Perform `entt::registry::try_get` on earlier registered component. */
    entt::meta_handle get(entt::entity entity, entt::meta_type component_type) const;

    /** Perform `entt::registry::get_or_assign` on earlier registered component. Default constructor is used. */
    entt::meta_handle get_or_assign(entt::entity entity, entt::meta_type component_type);

    /** Iterate over all registered components of specified `entity`. */
    template <typename T>
    void each_registered_component(entt::entity entity, T callback) const;

    /** Iterate over all editable components of specified `entity`. */
    template <typename T>
    void each_editable_component(entt::entity entity, T callback) const;

    /** Allow using entt versions of `assign`, `remove`, `has`, `get` and `get_or_assign` methods. */
    using entt::registry::remove;
    using entt::registry::has;
    using entt::registry::get;
    using entt::registry::get_or_assign;

    /** Remove all the tags from this world. */
    void clear_tags();

    /** Add specified tags to this world. */
    template <typename... Tags>
    void add_tags(Tags&&... tags);
    void add_tag(const char* tag);

    /** Remove the specified tags from this world. */
    template <typename... Tags>
    void remove_tags(Tags&&... tags);
    void remove_tag(const char* tag);

    /** Check whether the specified tags are added to this world. */
    template <typename... Tags>
    bool check_tags(Tags&&... tags);
    bool check_tag(const char* tag);

    /** Execute all systems of specified type. */
    bool update_normal(float elapsed_time);
    void update_fixed(float elapsed_time);

private:
    struct SystemInstance final {
        std::unique_ptr<System> instance;
        float construction_duration = 0.f;
    };

    enum class PropagateState : uint8_t {
        NOT_VISITED,
        IN_PROGRESS,
        COMPLETED,
    };

    void sort_systems(size_t system_type);
    bool check_tag_indices(const std::vector<size_t>& require, const std::vector<size_t>& exclusive);
    void propagate_system(size_t system_type, size_t system_index);

    std::vector<SystemInstance> m_systems[2];
    std::vector<size_t> m_system_order[2];
    std::vector<PropagateState> m_propagate_state[2];

    std::vector<uint8_t> m_tags;
    bool m_tags_changed[2]{};
};

/** Presence of this single component means the world may keep running. */
struct RunningWorldSingleComponent final {
    bool dummy = false;
};

} // namespace hg

#include "core/ecs/private/world_impl.h"
