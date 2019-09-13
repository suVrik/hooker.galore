#pragma once

#include <glm/vec3.hpp>

namespace hg {

/** `PhysicsBoxShapeComponent` adds a physical box shape to rigid body. */
struct PhysicsBoxShapeComponent final {
    glm::vec3 size = glm::vec3(1.f, 1.f, 1.f);
};

} // namespace hg
