#include "core/ecs/world.h"
#include "world/shared/render/render_system.h"

#include <bgfx/bgfx.h>

namespace hg {

RenderSystem::RenderSystem(World& world) noexcept
        : NormalSystem(world) {
}

void RenderSystem::update(float /*elapsed_time*/) {
    assert(world.after("RenderFetchSystem"));
    bgfx::frame();
}

} // namespace hg
