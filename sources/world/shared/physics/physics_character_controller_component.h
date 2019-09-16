#pragma once

#include <entt/entity/fwd.hpp>
#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>
#include <optional>

namespace hg {

class PhysicsCharacterControllerFetchSystem;
class PhysicsCharacterControllerSystem;
class World;

/** `PhysicsCharacterControllerComponent` makes the entity a character controller. */
class PhysicsCharacterControllerComponent final {
public:
    enum class NonWalkableMode {
        PREVENT_CLIMBING,                   ///< Stops character from climbing up non-walkable slopes, but doesn't move it otherwise.
        PREVENT_CLIMBING_AND_FORCE_SLIDING, ///< Stops character from climbing up non-walkable slopes, and forces it to slide down those slopes.
    };

    enum class ClimbingMode {
        EASY,        ///< Standard mode, let the capsule climb over surfaces according to impact normal.
        CONSTRAINED, ///< Constrained mode, try to limit climbing according to the step offset.
    };

    /** Get/Set character controller step offset. */
    float get_step_offset() const noexcept;
    void set_step_offset(float value) noexcept;

    /** Get/Set character controller non walkable mode. */
    NonWalkableMode get_non_walkable_mode() const noexcept;
    void set_non_walkable_mode(NonWalkableMode value) noexcept;

    /** Get/Set character controller contact offset. */
    float get_contact_offset() const noexcept;
    void set_contact_offset(float value) noexcept;

    /** Get/Set character controller up direction. */
    const glm::vec3& get_up_direction() const noexcept;
    void set_up_direction(const glm::vec3& value) noexcept;

    /** Get/Set character controller slope limit. */
    float get_slope_limit() const noexcept;
    void set_slope_limit(float value) noexcept;

    /** Get/Set character controller capsule radius. */
    float get_radius() const noexcept;
    void set_radius(float value) noexcept;

    /** Get/Set character controller capsule height. */
    float get_height() const noexcept;
    void set_height(float value) noexcept;

    /** Get/Set character controller capsule climbing mode. */
    ClimbingMode get_climbing_mode() const noexcept;
    void set_climbing_mode(ClimbingMode value) noexcept;

    /** Move character controller by specified `offset` with all the collision detection. */
    void move(const glm::vec3& offset) noexcept;

    /** Check whether the character controller is grounded. */
    bool is_grounded() const noexcept;

private:
    float m_step_offset = 0.5f;
    NonWalkableMode m_non_walkable_mode = NonWalkableMode::PREVENT_CLIMBING;
    float m_contact_offset = 0.1f;
    glm::vec3 m_up_direction = glm::vec3(0.f, 1.f, 0.f);
    float m_slope_limit = 0.707f;
    float m_radius = 0.25f;
    float m_height = 1.3f;
    ClimbingMode m_climbing_mode = ClimbingMode::EASY;
    bool m_descriptor_changed = false;

    std::optional<glm::vec3> m_offset = std::nullopt;
    bool m_is_grounded = false;

    friend class PhysicsCharacterControllerFetchSystem;
    friend class PhysicsCharacterControllerSystem;
};

} // namespace hg

#include "world/shared/physics/private/physics_character_controller_component_impl.h"
