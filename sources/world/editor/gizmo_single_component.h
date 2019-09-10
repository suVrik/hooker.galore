#pragma once

#include <glm/mat4x4.hpp>
#include <ImGuizmo.h>
#include <memory>

namespace hg {

/** `GizmoSingleComponent` contains currently selected gizmo operation. */
struct GizmoSingleComponent final {
    ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
    bool is_local_space = false;

    glm::mat4 transform;

    bool is_changing = false;

    // Menu items.
    std::shared_ptr<bool> switch_space;
    std::shared_ptr<bool> translate_tool;
    std::shared_ptr<bool> rotate_tool;
    std::shared_ptr<bool> scale_tool;
    std::shared_ptr<bool> bounds_tool;
};

} // namespace hg
