#include "world/shared/physics/physics_static_rigid_body_private_component.h"

#include <algorithm>
#include <cassert>

namespace hg {

PhysicsStaticRigidBodyPrivateComponent::PhysicsStaticRigidBodyPrivateComponent(PhysicsStaticRigidBodyPrivateComponent&& original) noexcept 
        : rigid_actor(original.rigid_actor) {
    original.rigid_actor = nullptr;
}

PhysicsStaticRigidBodyPrivateComponent::~PhysicsStaticRigidBodyPrivateComponent() {
    assert(rigid_actor == nullptr);
}

PhysicsStaticRigidBodyPrivateComponent& PhysicsStaticRigidBodyPrivateComponent::operator=(PhysicsStaticRigidBodyPrivateComponent&& original) noexcept {
    assert(this != &original);
    assert(rigid_actor == nullptr);

    rigid_actor = original.rigid_actor;
    original.rigid_actor = nullptr;

    return *this;
}

} // namespace hg
