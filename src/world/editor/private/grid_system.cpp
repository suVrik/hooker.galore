#include "core/ecs/world.h"
#include "core/render/debug_draw.h"
#include "world/editor/grid_single_component.h"
#include "world/editor/grid_system.h"
#include "world/editor/menu_single_component.h"

namespace hg {

GridSystem::GridSystem(World& world) noexcept
        : NormalSystem(world) {
    auto& grid_single_component = world.set<GridSingleComponent>();
    grid_single_component.is_shown = std::make_shared<bool>(true);

    auto& menu_single_component = world.ctx<MenuSingleComponent>();
    menu_single_component.items.emplace("2View/Grid", grid_single_component.is_shown);
}

void GridSystem::update(float /*elapsed_time*/) {
    const float GRID_SIZE = 10.f;
    const float GRID_STEP = 1.f;
    const glm::vec3 GRID_COLOR(0.75f, 0.75f, 0.75f);

    auto& grid_single_component = world.ctx<GridSingleComponent>();
    if (*grid_single_component.is_shown) {
        dd::xzSquareGrid(-GRID_SIZE, GRID_SIZE, 0.f, GRID_STEP, GRID_COLOR);
    }
}

} // namespace hg
