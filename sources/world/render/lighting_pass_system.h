#pragma once

#include "core/ecs/system.h"

namespace hg {

struct LightingPassSingleComponent;

/** `LightingPassSystem` applies directional light to all visible objects. */
class LightingPassSystem final : public NormalSystem {
public:
    explicit LightingPassSystem(World& world);
    void update(float elapsed_time) override;

private:
    void reset(LightingPassSingleComponent& lighting_pass_single_component, uint16_t width, uint16_t height);
};

} // namespace hg
