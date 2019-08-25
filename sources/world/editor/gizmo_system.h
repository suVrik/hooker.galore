#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `GizmoSystem` shows gizmo that allows to modify transform of selected object. */
class GizmoSystem final : public NormalSystem {
public:
    explicit GizmoSystem(World& world) noexcept;
    void update(float elapsed_time) override;
};

} // namespace hg
