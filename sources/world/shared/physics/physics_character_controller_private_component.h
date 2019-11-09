#pragma once

namespace physx {

class PxCapsuleController;

} // namespace physx

namespace hg {

class PhysicsCharacterControllerSystem;

/** `PhysicsCharacterControllerPrivateComponent` is added, updated and removed automatically by `PhysicsCharacterControllerSystem`.
    To make an entity a character controller use `PhysicsCharacterControllerComponent` instead. */
class PhysicsCharacterControllerPrivateComponent final {
public:
    PhysicsCharacterControllerPrivateComponent() = default;
    PhysicsCharacterControllerPrivateComponent(const PhysicsCharacterControllerPrivateComponent& original) = delete;
    PhysicsCharacterControllerPrivateComponent(PhysicsCharacterControllerPrivateComponent&& original);
    ~PhysicsCharacterControllerPrivateComponent();
    PhysicsCharacterControllerPrivateComponent& operator=(const PhysicsCharacterControllerPrivateComponent& original) = delete;
    PhysicsCharacterControllerPrivateComponent& operator=(PhysicsCharacterControllerPrivateComponent&& original);

private:
    physx::PxCapsuleController* m_controller = nullptr;

    friend class PhysicsCharacterControllerSystem;
};

} // namespace hg
