#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `PhysicsInitializationSystem` initializes physics engine and performs physics simulation. */
class PhysicsInitializationSystem final : public FixedSystem {
public:
    explicit PhysicsInitializationSystem(World& world);
    ~PhysicsInitializationSystem() override;
    void update(float elapsed_time) override;
};

} // namespace hg
