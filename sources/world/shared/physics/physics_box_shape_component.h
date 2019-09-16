#pragma once

#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

namespace hg {

/** `PhysicsBoxShapeComponent` adds a physical box shape to rigid body. */
class PhysicsBoxShapeComponent final {
public:
    /** Get/Set box shape size. */
    const glm::vec3& get_size() const noexcept;
    void set_size(const glm::vec3& value) noexcept;

private:
    glm::vec3 m_size = glm::vec3(1.f, 1.f, 1.f);
};

} // namespace hg

#include "world/shared/physics/private/physics_box_shape_component_impl.h"
