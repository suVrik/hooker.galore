#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `RenderSystem` displays a picture on the screen. */
class RenderSystem final : public NormalSystem {
public:
    explicit RenderSystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
