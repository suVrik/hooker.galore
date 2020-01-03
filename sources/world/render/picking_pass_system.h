#pragma once

#include "core/ecs/system.h"

namespace hg {

struct PickingPassSingleComponent;

/** `PickingPassSystem` performs picking pass for all objects when `perform_picking` is set to true and saves it into
    a texture stored in `PickingPassSingleComponent`. */
class PickingPassSystem final : public NormalSystem {
public:
    explicit PickingPassSystem(World& world);
    void update(float elapsed_time) override;

private:
    void reset(PickingPassSingleComponent& picking_pass_single_component, uint16_t width, uint16_t height);
};

} // namespace hg
