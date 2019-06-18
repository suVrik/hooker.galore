#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `LightingPassSystem` performs lighting pass after geometry pass. */
class LightingPassSystem final : public NormalSystem {
public:
    explicit LightingPassSystem(World& world);
    ~LightingPassSystem() override;
    void update(float elapsed_time) override;
};

} // namespace hg
