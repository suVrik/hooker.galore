#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/shared/physics/physics_fetch_system.h"
#include "world/shared/physics/physics_single_component.h"

#include <PxScene.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(PhysicsFetchSystem),
    REQUIRE("physics"),
    AFTER("PhysicsInitializationSystem")
)

PhysicsFetchSystem::PhysicsFetchSystem(World& world)
        : FixedSystem(world) {
}

void PhysicsFetchSystem::update(float /*elapsed_time*/) {
    auto& physics_single_component = world.ctx<PhysicsSingleComponent>();

    assert(physics_single_component.scene != nullptr);
    physics_single_component.scene->fetchResults(true);
}

} // namespace hg
