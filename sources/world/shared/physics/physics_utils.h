#pragma once

#include "world/shared/transform_component.h"

#include <characterkinematic/PxExtended.h>
#include <foundation/PxTransform.h>
#include <foundation/PxVec3.h>
#include <glm/ext/quaternion_float.hpp>
#include <glm/vec3.hpp>

namespace hg {

inline physx::PxVec3 glm_vec3_to_physx_vec3(const glm::vec3& value) noexcept {
    return physx::PxVec3(value.x, value.y, value.z);
}

inline glm::vec3 physx_vec3_to_glm_vec3(const physx::PxVec3& value) noexcept {
    return glm::vec3(value.x, value.y, value.z);
}

inline physx::PxExtendedVec3 glm_vec3_to_physx_extended(const glm::vec3& value) noexcept {
    return physx::PxExtendedVec3(static_cast<double>(value.x), static_cast<double>(value.y), static_cast<double>(value.z));
}

inline glm::vec3 physx_extended_to_glm_vec3(const physx::PxExtendedVec3& value) noexcept {
    return glm::vec3(static_cast<float>(value.x), static_cast<float>(value.y), static_cast<float>(value.z));
}

inline physx::PxQuat glm_quat_to_physx_quat(const glm::quat& value) noexcept {
    return physx::PxQuat(value.x, value.y, value.z, value.w);
}

inline glm::quat physx_quat_to_glm_quat(const physx::PxQuat& value) noexcept {
    return glm::quat(value.w, value.x, value.y, value.z);
}

inline physx::PxTransform transform_component_to_physx_transform(const TransformComponent& transform_component) noexcept {
    return physx::PxTransform(glm_vec3_to_physx_vec3(transform_component.translation), glm_quat_to_physx_quat(transform_component.rotation));
}

inline physx::PxTransform transform_component_to_physx_transform(const TransformComponent* const transform_component) noexcept {
    return transform_component != nullptr ? transform_component_to_physx_transform(*transform_component) : physx::PxTransform(physx::PxIdentity);
}

} // namespace hg
