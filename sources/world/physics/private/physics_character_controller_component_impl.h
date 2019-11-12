#pragma once

#include "world/physics/physics_character_controller_component.h"

namespace hg {

inline void PhysicsCharacterControllerComponent::set_step_offset(const float value) {
    m_step_offset = glm::clamp(value, glm::epsilon<float>(), 2.f * m_radius + m_height - glm::epsilon<float>());
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_step_offset() const {
    return m_step_offset;
}

inline void PhysicsCharacterControllerComponent::set_non_walkable_mode(const NonWalkableMode value) {
    assert(static_cast<uint32_t>(value) < 2);
    m_non_walkable_mode = value;
    m_descriptor_changed = true;
}

inline PhysicsCharacterControllerComponent::NonWalkableMode PhysicsCharacterControllerComponent::get_non_walkable_mode() const {
    return m_non_walkable_mode;
}

inline void PhysicsCharacterControllerComponent::set_contact_offset(const float value) {
    m_contact_offset = std::max(value, glm::epsilon<float>());
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_contact_offset() const {
    return m_contact_offset;
}

inline void PhysicsCharacterControllerComponent::set_up_direction(const glm::vec3& value) {
    m_up_direction = value;
    m_descriptor_changed = true;
}

inline const glm::vec3& PhysicsCharacterControllerComponent::get_up_direction() const {
    return m_up_direction;
}

inline void PhysicsCharacterControllerComponent::set_slope_limit(const float value) {
    m_slope_limit = std::max(value, 0.f);
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_slope_limit() const {
    return m_slope_limit;
}

inline void PhysicsCharacterControllerComponent::set_radius(const float value) {
    m_radius = std::max(value, glm::epsilon<float>());
    m_step_offset = glm::clamp(m_step_offset, glm::epsilon<float>(), 2.f * m_radius + m_height - glm::epsilon<float>());
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_radius() const {
    return m_radius;
}

inline void PhysicsCharacterControllerComponent::set_height(const float value) {
    m_height = std::max(value, glm::epsilon<float>());
    m_step_offset = glm::clamp(m_step_offset, glm::epsilon<float>(), 2.f * m_radius + m_height - glm::epsilon<float>());
    m_descriptor_changed = true;
}

inline float PhysicsCharacterControllerComponent::get_height() const {
    return m_height;
}

inline void PhysicsCharacterControllerComponent::set_climbing_mode(const ClimbingMode value) {
    assert(static_cast<uint32_t>(value) < 2);
    m_climbing_mode = value;
    m_descriptor_changed = true;
}

inline PhysicsCharacterControllerComponent::ClimbingMode PhysicsCharacterControllerComponent::get_climbing_mode() const {
    return m_climbing_mode;
}

inline void PhysicsCharacterControllerComponent::move(const glm::vec3& offset) {
    if (m_offset) {
        m_offset = *m_offset + offset;
    } else {
        m_offset = offset;
    }
}

inline bool PhysicsCharacterControllerComponent::is_grounded() const {
    return m_is_grounded;
}

} // namespace hg
