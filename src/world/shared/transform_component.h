#pragma once

#include <glm/ext/quaternion_float.hpp>
#include <glm/vec3.hpp>

namespace hg {

/** `TransformComponent` contains geometrical transformation of an object. */
struct TransformComponent final {
    glm::vec3 translation = glm::vec3(0.f);
    glm::quat rotation    = glm::quat(0.f, 0.f, 0.f, 1.f);
    glm::vec3 scale       = glm::vec3(1.f);
};

} // namespace hg
