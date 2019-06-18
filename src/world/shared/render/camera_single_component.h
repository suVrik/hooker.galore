#pragma once

#include "../../../../lib/entt/include/entt/entity/registry.hpp"
#include "../../../../lib/glm/include/glm/mat4x4.hpp"

namespace hg {

/** `CameraSingleComponent` points to an active camera entity. */
struct CameraSingleComponent final {
    entt::entity active_camera = entt::null;

    glm::mat4 view_matrix                    = glm::mat4(1.f);
    glm::mat4 projection_matrix              = glm::mat4(1.f);
    glm::mat4 view_projection_matrix         = glm::mat4(1.f);
    glm::mat4 inverse_view_matrix            = glm::mat4(1.f);
    glm::mat4 inverse_projection_matrix      = glm::mat4(1.f);
    glm::mat4 inverse_view_projection_matrix = glm::mat4(1.f);
};

} // namespace hg
