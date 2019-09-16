#pragma once

#include "world/shared/physics/physics_character_controller_component.h"

namespace hg {

inline void PhysicsCharacterControllerComponent::set_step_offset(const float value) noexcept {
    m_step_offset = glm::clamp(value, glm::epsilon<float>(), 2.f * m_radius + m_height - glm::epsilon<float>());
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_step_offset() const noexcept {
    return m_step_offset;
}

inline void PhysicsCharacterControllerComponent::set_non_walkable_mode(const NonWalkableMode value) noexcept {
    assert(static_cast<uint32_t>(value) < 2);
    m_non_walkable_mode = value;
    m_descriptor_changed = true;
}

inline PhysicsCharacterControllerComponent::NonWalkableMode PhysicsCharacterControllerComponent::get_non_walkable_mode() const noexcept {
    return m_non_walkable_mode;
}

inline void PhysicsCharacterControllerComponent::set_contact_offset(const float value) noexcept {
    m_contact_offset = std::max(value, glm::epsilon<float>());
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_contact_offset() const noexcept {
    return m_contact_offset;
}

inline void PhysicsCharacterControllerComponent::set_up_direction(const glm::vec3& value) noexcept {
    m_up_direction = value;
    m_descriptor_changed = true;
}

inline const glm::vec3& PhysicsCharacterControllerComponent::get_up_direction() const noexcept {
    return m_up_direction;
}

inline void PhysicsCharacterControllerComponent::set_slope_limit(const float value) noexcept {
    m_slope_limit = std::max(value, 0.f);
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_slope_limit() const noexcept {
    return m_slope_limit;
}

inline void PhysicsCharacterControllerComponent::set_radius(const float value) noexcept {
    m_radius = std::max(value, glm::epsilon<float>());
    m_step_offset = glm::clamp(m_step_offset, glm::epsilon<float>(), 2.f * m_radius + m_height - glm::epsilon<float>());
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_radius() const noexcept {
    return m_radius;
}

inline void PhysicsCharacterControllerComponent::set_height(const float value) noexcept {
    m_height = std::max(value, glm::epsilon<float>());
    m_step_offset = glm::clamp(m_step_offset, glm::epsilon<float>(), 2.f * m_radius + m_height - glm::epsilon<float>());
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_height() const noexcept {
    return m_height;
}

inline void PhysicsCharacterControllerComponent::set_climbing_mode(const ClimbingMode value) noexcept {
    assert(static_cast<uint32_t>(value) < 2);
    m_climbing_mode = value;
    m_descriptor_changed = true;
}

inline PhysicsCharacterControllerComponent::ClimbingMode PhysicsCharacterControllerComponent::get_climbing_mode() const noexcept {
    return m_climbing_mode;
}

inline void PhysicsCharacterControllerComponent::move(const glm::vec3& offset) noexcept {
    m_offset = offset;
}

inline bool PhysicsCharacterControllerComponent::is_grounded() const noexcept {
    return m_is_grounded;
}

} // namespace hg
