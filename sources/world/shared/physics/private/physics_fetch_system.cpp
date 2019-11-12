#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/shared/physics/physics_fetch_system.h"
#include "world/shared/physics/physics_single_component.h"
#include "world/shared/physics/physics_tags.h"

#include <PxScene.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(PhysicsFetchSystem),
    TAGS(physics),
    AFTER("PhysicsInitializationSystem")
)

PhysicsFetchSystem::PhysicsFetchSystem(World& world)
        : FixedSystem(world) {
}

void PhysicsFetchSystem::update(float /*elapsed_time*/) {
    auto& physics_single_component = world.ctx<PhysicsSingleComponent>();
    physics_single_component.get_scene().fetchResults(true);
}

} // namespace hg
