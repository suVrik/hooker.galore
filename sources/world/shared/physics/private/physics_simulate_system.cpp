#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/shared/physics/physics_simulate_system.h"
#include "world/shared/physics/physics_single_component.h"

#include <PxScene.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(PhysicsSimulateSystem),
    REQUIRE("physics"),
    BEFORE("PhysicsFetchSystem"),
    AFTER("PhysicsInitializationSystem")
)

PhysicsSimulateSystem::PhysicsSimulateSystem(World& world)
        : FixedSystem(world) {
}

void PhysicsSimulateSystem::update(float elapsed_time) {
    auto& physics_single_component = world.ctx<PhysicsSingleComponent>();

    assert(physics_single_component.scene != nullptr);
    physics_single_component.scene->simulate(elapsed_time);
}

} // namespace hg
