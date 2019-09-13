#pragma once

namespace physx {

class PxShape;

} // namespace physx

namespace hg {

/** `PhysicsBoxShapePrivateComponent` is added, updated and removed automatically by `PhysicsShapeSystem`.
    To add a physical box shape to entity use `PhysicsBoxShapeComponent` instead. */
struct PhysicsBoxShapePrivateComponent final {
    PhysicsBoxShapePrivateComponent() = default;
    PhysicsBoxShapePrivateComponent(const PhysicsBoxShapePrivateComponent& original) = delete;
    PhysicsBoxShapePrivateComponent(PhysicsBoxShapePrivateComponent&& original) noexcept;
    ~PhysicsBoxShapePrivateComponent();
    PhysicsBoxShapePrivateComponent& operator=(const PhysicsBoxShapePrivateComponent& original) = delete;
    PhysicsBoxShapePrivateComponent& operator=(PhysicsBoxShapePrivateComponent&& original) noexcept;

    physx::PxShape* shape = nullptr;
};

} // namespace hg
