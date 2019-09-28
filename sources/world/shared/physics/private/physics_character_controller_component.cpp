#include "core/meta/registration.h"
#include "world/shared/physics/physics_character_controller_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<PhysicsCharacterControllerComponent>("PhysicsCharacterControllerComponent")
            .data<&PhysicsCharacterControllerComponent::set_step_offset, &PhysicsCharacterControllerComponent::get_step_offset>("step_offset")
            .data<&PhysicsCharacterControllerComponent::set_non_walkable_mode, &PhysicsCharacterControllerComponent::get_non_walkable_mode>("non_walkable_mode")
            .data<&PhysicsCharacterControllerComponent::set_contact_offset, &PhysicsCharacterControllerComponent::get_contact_offset>("contact_offset")
            .data<&PhysicsCharacterControllerComponent::set_up_direction, &PhysicsCharacterControllerComponent::get_up_direction>("up_direction")
            .data<&PhysicsCharacterControllerComponent::set_slope_limit, &PhysicsCharacterControllerComponent::get_slope_limit>("slope_limit")
            .data<&PhysicsCharacterControllerComponent::set_radius, &PhysicsCharacterControllerComponent::get_radius>("radius")
            .data<&PhysicsCharacterControllerComponent::set_height, &PhysicsCharacterControllerComponent::get_height>("height")
            .data<&PhysicsCharacterControllerComponent::set_climbing_mode, &PhysicsCharacterControllerComponent::get_climbing_mode>("climbing_mode");
}

} // namespace hg
