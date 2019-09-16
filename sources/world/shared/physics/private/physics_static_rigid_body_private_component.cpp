#include "world/shared/physics/physics_static_rigid_body_private_component.h"

#include <cassert>

namespace hg {

PhysicsStaticRigidBodyPrivateComponent::PhysicsStaticRigidBodyPrivateComponent(PhysicsStaticRigidBodyPrivateComponent&& original) noexcept 
        : m_rigid_actor(original.m_rigid_actor) {
    original.m_rigid_actor = nullptr;
}

PhysicsStaticRigidBodyPrivateComponent::~PhysicsStaticRigidBodyPrivateComponent() {
    assert(m_rigid_actor == nullptr);
}

PhysicsStaticRigidBodyPrivateComponent& PhysicsStaticRigidBodyPrivateComponent::operator=(PhysicsStaticRigidBodyPrivateComponent&& original) noexcept {
    assert(this != &original);
    assert(m_rigid_actor == nullptr);

    m_rigid_actor = original.m_rigid_actor;
    original.m_rigid_actor = nullptr;

    return *this;
}

} // namespace hg
