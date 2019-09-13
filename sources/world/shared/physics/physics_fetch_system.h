#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `PhysicsFetchSystem` waits until end of PhysX simulation and fetches data from it. */
class PhysicsFetchSystem final : public FixedSystem {
public:
    explicit PhysicsFetchSystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
