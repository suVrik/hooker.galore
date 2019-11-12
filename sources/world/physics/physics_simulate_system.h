#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `PhysicsSimulateSystem` performs physics simulation. */
class PhysicsSimulateSystem final : public FixedSystem {
public:
    explicit PhysicsSimulateSystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
