#pragma once

#include <glm/vec3.hpp>

namespace hg {

/** `LightComponent` describes a light source. */
struct LightComponent final {
    glm::vec3 color = glm::vec3(150.f, 150.f, 150.f);
};

} // namespace hg
