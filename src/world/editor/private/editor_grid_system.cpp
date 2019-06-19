#include "world/editor/editor_grid_system.h"
#include "core/render/debug_draw.h"

namespace hg {

EditorGridSystem::EditorGridSystem(World& world) noexcept
        : NormalSystem(world) {
}

void EditorGridSystem::update(float /*elapsed_time*/) {
    const float GRID_SIZE = 10.f;
    const float GRID_STEP = 1.f;
    const glm::vec3 GRID_COLOR(0.75f, 0.75f, 0.75f);
    dd::xzSquareGrid(-GRID_SIZE, GRID_SIZE, 0.f, GRID_STEP, GRID_COLOR);
}

} // namespace hg
