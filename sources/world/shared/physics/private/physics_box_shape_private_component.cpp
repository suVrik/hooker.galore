#include "world/shared/physics/physics_box_shape_private_component.h"

#include <algorithm>
#include <cassert>

namespace hg {

PhysicsBoxShapePrivateComponent::PhysicsBoxShapePrivateComponent(PhysicsBoxShapePrivateComponent&& original) noexcept
        : shape(original.shape) {
    original.shape = nullptr;
}

PhysicsBoxShapePrivateComponent::~PhysicsBoxShapePrivateComponent() {
    assert(shape == nullptr);
}

PhysicsBoxShapePrivateComponent& PhysicsBoxShapePrivateComponent::operator=(PhysicsBoxShapePrivateComponent&& original) noexcept {
    assert(this != &original);
    assert(shape == nullptr);

    shape = original.shape;
    original.shape = nullptr;

    return *this;
}

} // namespace hg
