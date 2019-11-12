#include "world/physics/physics_character_controller_private_component.h"

#include <cassert>

namespace hg {

PhysicsCharacterControllerPrivateComponent::PhysicsCharacterControllerPrivateComponent(PhysicsCharacterControllerPrivateComponent&& original)
        : m_controller(original.m_controller) {
    original.m_controller = nullptr;
}

PhysicsCharacterControllerPrivateComponent::~PhysicsCharacterControllerPrivateComponent() {
    assert(m_controller == nullptr);
}

PhysicsCharacterControllerPrivateComponent& PhysicsCharacterControllerPrivateComponent::operator=(PhysicsCharacterControllerPrivateComponent&& original) {
    assert(this != &original);
    assert(m_controller == nullptr);

    m_controller = original.m_controller;
    original.m_controller = nullptr;

    return *this;
}

} // namespace hg
