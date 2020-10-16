#include "core/ecs/system_descriptor.h"
#include "world/resource/resource_single_component.h"
#include "world/resource/resource_system.h"

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(NewResourceSystem),
    CONTEXT(ResourceSingleComponent)
)

NewResourceSystem::NewResourceSystem(World& world) 
        : NormalSystem(world) {
    // TODO: Implementation.
}

void NewResourceSystem::update(float elapsed_time) {
    // TODO: Implementation.
}

} // namespace hg
