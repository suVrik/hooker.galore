#include "core/ecs/system.h"

namespace hg {

System::System(World& world)
        : world(world) {
}

System::~System() = default;

NormalSystem::NormalSystem(World& world)
        : System(world) {
}

NormalSystem::~NormalSystem() = default;

FixedSystem::FixedSystem(World& world)
        : System(world) {
}

FixedSystem::~FixedSystem() = default;

} // namespace hg
