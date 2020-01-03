#pragma once

#include "core/ecs/system.h"

namespace hg {

struct AAPassSingleComponent;

/** `AAPassSystem` applies anti-aliasing to the final image. */
class AAPassSystem final : public NormalSystem {
public:
    explicit AAPassSystem(World& world);
    ~AAPassSystem() override;
    void update(float elapsed_time) override;

private:
    void reset(AAPassSingleComponent& aa_pass_single_component, uint16_t width, uint16_t height);
};

} // namespace hg
