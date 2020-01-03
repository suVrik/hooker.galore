#pragma once

#include "core/ecs/system.h"

namespace hg {

struct OutlinePassSingleComponent;

/** `OutlinePassSystem` draws outline for all objects with `OutlineComponent` and presents it on the screen. */
class OutlinePassSystem final : public NormalSystem {
public:
    explicit OutlinePassSystem(World& world);
    void update(float elapsed_time) override;

private:
    void reset(OutlinePassSingleComponent& outline_pass_single_component, uint16_t width, uint16_t height);
};

} // namespace hg
