#pragma once

#include <entt/entity/registry.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/mat4x4.hpp>

namespace hg {

/** `CameraSingleComponent` points to an active camera entity. */
struct CameraSingleComponent final {
    entt::entity active_camera = entt::null;

    glm::vec3 translation                    = glm::vec3(0.f);
    glm::quat rotation                       = glm::quat(1.f, 0.f, 0.f, 0.f);
    glm::mat4 view_matrix                    = glm::mat4(1.f);
    glm::mat4 projection_matrix              = glm::mat4(1.f);
    glm::mat4 view_projection_matrix         = glm::mat4(1.f);
    glm::mat4 inverse_view_matrix            = glm::mat4(1.f);
    glm::mat4 inverse_projection_matrix      = glm::mat4(1.f);
    glm::mat4 inverse_view_projection_matrix = glm::mat4(1.f);
};

} // namespace hg
