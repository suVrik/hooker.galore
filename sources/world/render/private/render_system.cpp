#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/render/render_single_component.h"
#include "world/render/render_system.h"
#include "world/render/render_tags.h"

#include <bgfx/bgfx.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(RenderSystem),
    TAGS(render),
    AFTER("RenderFetchSystem")
)

RenderSystem::RenderSystem(World& world)
        : NormalSystem(world) {
    world.set<RenderSingleComponent>();
}

void RenderSystem::update(float /*elapsed_time*/) {
    world.ctx<RenderSingleComponent>().current_frame = bgfx::frame();
}

} // namespace hg
