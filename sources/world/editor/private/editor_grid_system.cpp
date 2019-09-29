#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/editor/editor_grid_single_component.h"
#include "world/editor/editor_grid_system.h"
#include "world/editor/editor_menu_single_component.h"

#include <debug_draw.hpp>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(EditorGridSystem),
    REQUIRE("editor"),
    BEFORE("DebugDrawPassSystem"),
    AFTER("EditorMenuSystem")
)

EditorGridSystem::EditorGridSystem(World& world) noexcept
        : NormalSystem(world) {
    auto& editor_grid_single_component = world.set<EditorGridSingleComponent>();
    editor_grid_single_component.is_shown = std::make_shared<bool>(true);

    auto& editor_menu_single_component = world.ctx<EditorMenuSingleComponent>();
    editor_menu_single_component.add_item("2View/Grid", editor_grid_single_component.is_shown);
}

void EditorGridSystem::update(float /*elapsed_time*/) {
    const float GRID_SIZE = 10.f;
    const float GRID_STEP = 1.f;
    const glm::vec3 GRID_COLOR(0.75f, 0.75f, 0.75f);

    auto& editor_grid_single_component = world.ctx<EditorGridSingleComponent>();
    if (*editor_grid_single_component.is_shown) {
        dd::xzSquareGrid(-GRID_SIZE, GRID_SIZE, 0.f, GRID_STEP, GRID_COLOR);
    }
}

} // namespace hg
