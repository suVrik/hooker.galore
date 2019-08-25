#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `PhysicsFetchSystem` fetches data from physics engine and puts it into the world. */
// TODO: Actually `PhysicsFetchSystem` is a `FixedSystem`. Keep it `NormalSystem` until physics is stable.
class PhysicsFetchSystem final : public NormalSystem {
public:
    explicit PhysicsFetchSystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
