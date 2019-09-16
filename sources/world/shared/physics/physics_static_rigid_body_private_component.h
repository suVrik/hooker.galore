#pragma once

namespace physx {

class PxRigidStatic;

} // namespace physx

namespace hg {

class PhysicsRigidBodySystem;
class PhysicsShapeSystem;

/** `PhysicsStaticRigidBodyPrivateComponent` is added, updated and removed automatically by `PhysicsRigidBodySystem`.
    To mark an entity as static rigid body use `PhysicsStaticRigidBodyComponent` instead. */
class PhysicsStaticRigidBodyPrivateComponent final {
public:
    PhysicsStaticRigidBodyPrivateComponent() = default;
    PhysicsStaticRigidBodyPrivateComponent(const PhysicsStaticRigidBodyPrivateComponent& original) = delete;
    PhysicsStaticRigidBodyPrivateComponent(PhysicsStaticRigidBodyPrivateComponent&& original) noexcept;
    ~PhysicsStaticRigidBodyPrivateComponent();
    PhysicsStaticRigidBodyPrivateComponent& operator=(const PhysicsStaticRigidBodyPrivateComponent& original) = delete;
    PhysicsStaticRigidBodyPrivateComponent& operator=(PhysicsStaticRigidBodyPrivateComponent&& original) noexcept;

private:
    physx::PxRigidStatic* m_rigid_actor = nullptr;

    friend class PhysicsRigidBodySystem;
    friend class PhysicsShapeSystem;
};

} // namespace hg
