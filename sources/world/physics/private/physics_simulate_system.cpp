#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/physics/physics_simulate_system.h"
#include "world/physics/physics_single_component.h"
#include "world/physics/physics_tags.h"

#include <PxScene.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(PhysicsSimulateSystem),
    TAGS(physics),
    BEFORE("PhysicsFetchSystem"),
    AFTER("PhysicsInitializationSystem")
)

PhysicsSimulateSystem::PhysicsSimulateSystem(World& world)
        : FixedSystem(world) {
}

void PhysicsSimulateSystem::update(float elapsed_time) {
    auto& physics_single_component = world.ctx<PhysicsSingleComponent>();
    physics_single_component.get_scene().simulate(elapsed_time);
}

} // namespace hg
