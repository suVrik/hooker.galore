#pragma once

namespace physx {

class PxControllerManager;

} // namespace physx

namespace hg {

class PhysicsCharacterControllerSystem;

/** `PhysicsCharacterControllerPrivateComponent` is added, updated and removed automatically by `PhysicsCharacterControllerSystem`.
    To make an entity a character controller use `PhysicsCharacterControllerComponent` instead. */
class PhysicsCharacterControllerSingleComponent final {
private:
    physx::PxControllerManager* m_character_controller_manager = nullptr;

    friend class PhysicsCharacterControllerSystem;
};

} // namespace hg
