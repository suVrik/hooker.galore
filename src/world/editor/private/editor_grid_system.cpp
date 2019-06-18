#include "world/editor/editor_grid_system.h"
#include "core/render/debug_draw.h"

#include "core/ecs/world.h"
#include "world/shared/render/camera_single_component.h"
#include <glm/gtc/type_ptr.hpp>
#include <world/shared/window_single_component.h>

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
