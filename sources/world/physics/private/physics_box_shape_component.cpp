#include "core/meta/registration.h"
#include "world/physics/physics_box_shape_component.h"

#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<PhysicsBoxShapeComponent>("PhysicsBoxShapeComponent")
            .data<&PhysicsBoxShapeComponent::set_size, &PhysicsBoxShapeComponent::get_size>("size");
}

} // namespace hg
