#pragma once

#include <entt/entity/registry.hpp>

namespace hg {

class World;

/** `SelectedEntitySingleComponent` contains selected entity. */
class SelectedEntitySingleComponent final {
public:
    /** Select specified `entity`, assign `OutlineComponent` to it, reset `OutlineComponent` from previous selected entity. */
    void select_entity(World& world, entt::entity entity) noexcept;

    entt::entity selected_entity = entt::null;

    bool is_selecting = false;
    int32_t selection_start_x = 0;
    int32_t selection_start_y = 0;
    int32_t selection_end_x = 0;
    int32_t selection_end_y = 0;

    bool waiting_for_pick = false;
};

} // namespace hg
