#include "core/meta/registration.h"
#include "world/shared/physics/physics_box_shape_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<PhysicsBoxShapeComponent>("PhysicsBoxShapeComponent")
            .data<&PhysicsBoxShapeComponent::size>("size");
}

} // namespace hg
