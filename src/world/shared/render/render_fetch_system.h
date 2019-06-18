#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `RenderFetchSystem` prepares renderer for draw requests. */
class RenderFetchSystem final : public NormalSystem {
public:
    explicit RenderFetchSystem(World& world);
    ~RenderFetchSystem() override;
    void update(float elapsed_time) override;
};

} // namespace hg
