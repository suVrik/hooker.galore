#pragma once

#include "core/ecs/system.h"

namespace hg {

struct SkyboxPassSingleComponent;

/** `SkyboxPassSystem` merges the skybox with the result of lighting pass. */
class SkyboxPassSystem final : public NormalSystem {
public:
    explicit SkyboxPassSystem(World& world);
    void update(float elapsed_time) override;

private:
    void reset(SkyboxPassSingleComponent& lighting_pass_single_component, uint16_t width, uint16_t height);
};

} // namespace hg
