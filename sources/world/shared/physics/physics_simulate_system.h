#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `PhysicsSimulateSystem` initializes physics engine and performs physics simulation. */
// TODO: Actually `PhysicsSimulateSystem` is a `FixedSystem`. Keep it `NormalSystem` until physics is stable.
class PhysicsSimulateSystem final : public NormalSystem {
public:
    explicit PhysicsSimulateSystem(World& world);
    ~PhysicsSimulateSystem() override;
    void update(float elapsed_time) override;
};

} // namespace hg
