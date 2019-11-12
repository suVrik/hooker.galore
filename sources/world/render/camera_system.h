#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `CameraSystem` updates camera matrices in `CameraSingleComponent`. */
class CameraSystem final : public NormalSystem {
public:
    explicit CameraSystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
