#pragma once

#include <entt/entity/registry.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/mat4x4.hpp>

namespace hg {

/** `CameraSingleComponent` references the active camera entity and contains information about its matrices. */
struct CameraSingleComponent final {
    entt::entity active_camera = entt::null;

    glm::vec3 translation                    = glm::vec3(0.f);
    glm::quat rotation                       = glm::quat(1.f, 0.f, 0.f, 0.f);
    float fov                                = glm::radians(60.f);
    float z_near                             = 0.1f;
    float z_far                              = 1000.f;
    glm::mat4 view_matrix                    = glm::mat4(1.f);
    glm::mat4 projection_matrix              = glm::mat4(1.f);
    glm::mat4 view_projection_matrix         = glm::mat4(1.f);
    glm::mat4 inverse_view_matrix            = glm::mat4(1.f);
    glm::mat4 inverse_projection_matrix      = glm::mat4(1.f);
    glm::mat4 inverse_view_projection_matrix = glm::mat4(1.f);
};

} // namespace hg
