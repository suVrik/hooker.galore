#pragma once

#include "core/ecs/component_manager.h"
#include "core/ecs/tags.h"

#include <entt/entity/registry.hpp>
#include <entt/meta/factory.hpp>
#include <string>
#include <vector>

namespace hg {

class System;

/** `World` is an extension over `entt::registry` that allows to work with components without knowing their compile time
    type and allows to manage and run ECS systems. A world can extend another world (they're called a child and a parent
    world respectively). A child world may inherit some tags from its parent automatically (depends on tag's settings)
    as well as propagate its own tags to it (depends on tag's settings as well). Child world has access to parent's
    single components automatically using "set" and "ctx" methods, but not the other way around. */
class World final : public entt::registry {
public:
    /** Construct world. Pass nullptr to construct a root world. Child world otherwise. */
    explicit World(World* parent = nullptr);

    /** World reference is cached in systems and other places, so don't allow to copy or move the world.
        Smart pointer is recommended if you want to store your world in some movable structure. */
    World(const World& original) = delete;
    World(World&& original) = delete;
    ~World();
    World& operator=(const World& original) = delete;
    World& operator=(World&& original) = delete;

    /** Return parent world. Root world returns nullptr. */
    World* get_parent() const;

    /// SINGLE COMPONENTS ////////////////////////////////////////////////////

    /** Perform `entt::registry::try_ctx`, but if single component with specified type doesn't exist in this world,
        check it in parent world as well (and so on through hierarchy). */
    template <typename T>
    const T* try_ctx() const;
    template <typename T>
    T* try_ctx();

    /** Perform `entt::registry::ctx`, but if single component with specified type doesn't exist in this world,
        check it in parent world as well (and so on through hierarchy). */
    template <typename T>
    const T& ctx() const;
    template <typename T>
    T& ctx();

    /** Perform `try_ctx` on earlier registered single component. */
    entt::meta_handle ctx(entt::meta_type single_component_type) const;

    /** Check whether single component with specified type exists. */
    template <typename T>
    bool has_ctx() const;

    /** Perform `has_ctx` on earlier registered single component. */
    bool has_ctx(entt::meta_type single_component_type) const;

    /** Check whether specified single component type is owned by this world. */
    template <typename T>
    bool is_owned_ctx() const;

    /** Perform `is_owned_ctx` on earlier registered single component. */
    bool is_owned_ctx(entt::meta_type single_component_type) const;

    /** Iterate over all registered single components of this world.

        world.each_registered_single_component([](const entt::meta_handle component_handle) {
            // Your code goes here
        }); */
    template <typename T>
    void each_registered_single_component(T callback) const;

    /// COMPONENTS ///////////////////////////////////////////////////////////

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

    /** Iterate over all registered components of specified `entity`.

        world.each_registered_component(entity, [](const entt::meta_handle component_handle) {
            // Your code goes here
        }); */
    template <typename T>
    void each_registered_component(entt::entity entity, T callback) const;

    /** Iterate over all editable components of specified `entity`. 

        world.each_editable_component(entity, [](const entt::meta_handle component_handle) {
            // Your code goes here
        }); */
    template <typename T>
    void each_editable_component(entt::entity entity, T callback) const;

    /** Allow using entt versions of `assign`, `remove`, `has`, `get` and `get_or_assign` methods. */
    using entt::registry::remove;
    using entt::registry::has;
    using entt::registry::get;
    using entt::registry::get_or_assign;

    /// TAGS /////////////////////////////////////////////////////////////////

    /** Remove all the owned tags from this world. */
    void clear_tags();

    /** Add specified tags to this world. */
    template <typename... Tags>
    void add_tags(Tags&&... tags);
    void add_tag(Tag tag);

    /** Remove specified tags from this world. */
    template <typename... Tags>
    void remove_tags(Tags&&... tags);
    void remove_tag(Tag tag);

    /** Check whether the specified tags are added to this world. */
    template <typename... Tags>
    bool check_tags(Tags&&... tags) const;
    bool check_tag(Tag tag) const;

    /** Check whether the specified tag is owned by this world. */
    bool check_owned_tag(Tag tag) const;

    /** Iterate over all world tags.

        world.each_tag([](const Tag tag) {
            // Your code goes here
        }); */
    template <typename T>
    void each_tag(T callback) const;

    /** Iterate over all owned world tags.

        world.each_owned_tag([](const Tag tag) {
            // Your code goes here
        }); */
    template <typename T>
    void each_owned_tag(T callback) const;

    /// EXECUTION ////////////////////////////////////////////////////////////

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

    static constexpr size_t NORMAL = 0;
    static constexpr size_t FIXED = 1;

    void inherit_add_tag(Tag tag);
    void inherit_remove_tag(Tag tag);
    void propagate_add_tag(const World* child_world, Tag tag);
    void propagate_remove_tag(const World* child_world, Tag tag);
    void sort_systems(size_t system_type);
    void propagate_system(size_t system_type, size_t system_index);

    World* const m_parent;
    std::vector<World*> m_children;

    std::vector<SystemInstance> m_systems[2];
    std::vector<size_t> m_system_order[2];
    std::vector<PropagateState> m_propagate_state[2];

    std::vector<bool> m_owned_tags;
    std::vector<bool> m_inherited_tags;
    std::vector<size_t> m_propagated_tags;
    std::vector<bool> m_all_tags;
    bool m_tags_changed[2]{};
};

/** Presence of this single component means the world may keep running. */
struct RunningWorldSingleComponent final {
    bool dummy = false;
};

} // namespace hg

#include "core/ecs/private/world_impl.h"
