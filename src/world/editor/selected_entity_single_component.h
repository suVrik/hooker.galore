#pragma once

#include <entt/entity/registry.hpp>

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

    std::vector<entt::entity> selected_entities;

    bool is_selecting = false;
    int32_t selection_start_x = 0;
    int32_t selection_start_y = 0;
    int32_t selection_end_x = 0;
    int32_t selection_end_y = 0;

    bool waiting_for_pick = false;
};

} // namespace hg
