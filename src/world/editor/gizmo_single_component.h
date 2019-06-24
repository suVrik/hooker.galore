#pragma once

#include "core/render/ImGuizmo.h"

#include <glm/mat4x4.hpp>

namespace hg {

/** `GizmoSingleComponent` contains currently selected gizmo operation. */
class GizmoSingleComponent final {
public:
    ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
    bool is_local_space = false;

    glm::mat4 transform;

    bool is_changing = false;
};

} // namespace hg
