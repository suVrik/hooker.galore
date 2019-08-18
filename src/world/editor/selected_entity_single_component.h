#pragma once

#include <entt/entity/registry.hpp>
#include <memory>

namespace hg {

class World;

/** `SelectedEntitySingleComponent` contains selected entity. */
class SelectedEntitySingleComponent final {
public:
    /** Select specified `entity`, assign `OutlineComponent` to it, reset `OutlineComponent` from previous selected entity. */
    void select_entity(World& world, entt::entity entity) noexcept;
    void add_to_selection(World& world, entt::entity entity) noexcept;
    void remove_from_selection(World& world, entt::entity entity) noexcept;
    void clear_selection(World& world) noexcept;

    // Entity selection system.
    std::vector<entt::entity> selected_entities;
    int32_t selection_x     = 0;
    int32_t selection_y     = 0;
    uint32_t selection_time = 0;
    bool waiting_for_pick   = false;

    // Property editor system.
    entt::meta_type selected_component_to_add;

    // Menu items.
    std::shared_ptr<bool> select_all_entities;
    std::shared_ptr<bool> clear_selected_entities;
    std::shared_ptr<bool> delete_selected_entities;
};

} // namespace hg
