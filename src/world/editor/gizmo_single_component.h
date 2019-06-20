#pragma once

#include "core/render/ImGuizmo.h"

namespace hg {

/** `GizmoSingleComponent` contains currently selected gizmo operation. */
class GizmoSingleComponent final {
public:
    ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
    bool is_local_space = false;
};

} // namespace hg
