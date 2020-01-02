#pragma once

namespace physx {

class PxControllerManager;

} // namespace physx

namespace hg {

class PhysicsCharacterControllerSystem;

/** `PhysicsCharacterControllerSingleComponent` contains a character controller manager which manages an array of
    character controllers on a scene and which is required to create a new character controller on that scene. */
class PhysicsCharacterControllerSingleComponent final {
private:
    physx::PxControllerManager* m_character_controller_manager = nullptr;

    friend class PhysicsCharacterControllerSystem;
};

} // namespace hg
