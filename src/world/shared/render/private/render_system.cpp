#include "core/ecs/world.h"
#include "world/shared/render/render_single_component.h"
#include "world/shared/render/render_system.h"

#include <bgfx/bgfx.h>

namespace hg {

RenderSystem::RenderSystem(World& world) noexcept
        : NormalSystem(world) {
    world.set<RenderSingleComponent>();
}

void RenderSystem::update(float /*elapsed_time*/) {
    world.ctx<RenderSingleComponent>().current_frame = bgfx::frame();
}

} // namespace hg
