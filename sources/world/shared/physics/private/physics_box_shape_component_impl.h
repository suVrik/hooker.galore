#include "world/shared/physics/physics_box_shape_component.h"

namespace hg {

inline const glm::vec3& PhysicsBoxShapeComponent::get_size() const noexcept {
    return m_size;
}

inline void PhysicsBoxShapeComponent::set_size(const glm::vec3& value) noexcept {
    m_size = glm::max(value, glm::vec3(2.f * glm::epsilon<float>(), 2.f * glm::epsilon<float>(), 2.f * glm::epsilon<float>()));
}

} // namespace hg
