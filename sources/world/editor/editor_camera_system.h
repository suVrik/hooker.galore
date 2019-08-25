#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `EditorCameraSystem` updates camera in editor. */
class EditorCameraSystem final : public NormalSystem {
public:
    explicit EditorCameraSystem(World& world) noexcept;
    void update(float elapsed_time) override;
};

} // namespace hg
