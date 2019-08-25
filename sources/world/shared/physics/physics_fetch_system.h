#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `PhysicsFetchSystem` TODO: DESCRIPTION. */
// TODO: Actually `PhysicsFetchSystem` is a `FixedSystem`. Keep it `NormalSystem` until physics is stable.
class PhysicsFetchSystem final : public NormalSystem {
public:
    explicit PhysicsFetchSystem(World& world);
    ~PhysicsFetchSystem() override;
    void update(float elapsed_time) override;
};

} // namespace hg
