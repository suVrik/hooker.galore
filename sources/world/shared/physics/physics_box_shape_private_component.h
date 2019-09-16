#pragma once

namespace physx {

class PxShape;

} // namespace physx

namespace hg {

class PhysicsRigidBodySystem;
class PhysicsShapeSystem;

/** `PhysicsBoxShapePrivateComponent` is added, updated and removed automatically by `PhysicsShapeSystem`.
    To add a physical box shape to entity use `PhysicsBoxShapeComponent` instead. */
class PhysicsBoxShapePrivateComponent final {
public:
    PhysicsBoxShapePrivateComponent() = default;
    PhysicsBoxShapePrivateComponent(const PhysicsBoxShapePrivateComponent& original) = delete;
    PhysicsBoxShapePrivateComponent(PhysicsBoxShapePrivateComponent&& original) noexcept;
    ~PhysicsBoxShapePrivateComponent();
    PhysicsBoxShapePrivateComponent& operator=(const PhysicsBoxShapePrivateComponent& original) = delete;
    PhysicsBoxShapePrivateComponent& operator=(PhysicsBoxShapePrivateComponent&& original) noexcept;

private:
    physx::PxShape* m_shape = nullptr;

    friend class PhysicsRigidBodySystem;
    friend class PhysicsShapeSystem;
};

} // namespace hg
