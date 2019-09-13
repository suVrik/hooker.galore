#pragma once

namespace physx {

class PxRigidStatic;

} // namespace physx

namespace hg {

/** `PhysicsStaticRigidBodyPrivateComponent` is added, updated and removed automatically by `PhysicsRigidBodySystem`.
    To mark an entity as static rigid body use `PhysicsStaticRigidBodyComponent` instead. */
struct PhysicsStaticRigidBodyPrivateComponent final {
    PhysicsStaticRigidBodyPrivateComponent() = default;
    PhysicsStaticRigidBodyPrivateComponent(const PhysicsStaticRigidBodyPrivateComponent& original) = delete;
    PhysicsStaticRigidBodyPrivateComponent(PhysicsStaticRigidBodyPrivateComponent&& original) noexcept;
    ~PhysicsStaticRigidBodyPrivateComponent();
    PhysicsStaticRigidBodyPrivateComponent& operator=(const PhysicsStaticRigidBodyPrivateComponent& original) = delete;
    PhysicsStaticRigidBodyPrivateComponent& operator=(PhysicsStaticRigidBodyPrivateComponent&& original) noexcept;

    physx::PxRigidStatic* rigid_actor = nullptr;
};

} // namespace hg
