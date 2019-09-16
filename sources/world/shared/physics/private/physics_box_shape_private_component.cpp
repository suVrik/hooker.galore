#include "world/shared/physics/physics_box_shape_private_component.h"

#include <cassert>

namespace hg {

PhysicsBoxShapePrivateComponent::PhysicsBoxShapePrivateComponent(PhysicsBoxShapePrivateComponent&& original) noexcept
        : m_shape(original.m_shape) {
    original.m_shape = nullptr;
}

PhysicsBoxShapePrivateComponent::~PhysicsBoxShapePrivateComponent() {
    assert(m_shape == nullptr);
}

PhysicsBoxShapePrivateComponent& PhysicsBoxShapePrivateComponent::operator=(PhysicsBoxShapePrivateComponent&& original) noexcept {
    assert(this != &original);
    assert(m_shape == nullptr);

    m_shape = original.m_shape;
    original.m_shape = nullptr;

    return *this;
}

} // namespace hg
