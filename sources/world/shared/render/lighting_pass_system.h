#pragma once

#include "core/ecs/system.h"

namespace hg {

struct LightingPassSingleComponent;

/** `LightingPassSystem` performs lighting pass after geometry pass. */
class LightingPassSystem final : public NormalSystem {
public:
    explicit LightingPassSystem(World& world);
    ~LightingPassSystem() override;
    void update(float elapsed_time) override;
    void reset(LightingPassSingleComponent& lighting_pass_single_component, uint16_t width, uint16_t height) const;
};

} // namespace hg
