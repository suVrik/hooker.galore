#include "core/ecs/system.h"

namespace hg {

System::System(World& world) noexcept
        : world(world) {
}

System::~System() = default;

NormalSystem::NormalSystem(World& world) noexcept
        : System(world) {
}

NormalSystem::~NormalSystem() = default;

FixedSystem::FixedSystem(World& world) noexcept
        : System(world) {
}

FixedSystem::~FixedSystem() = default;

} // namespace hg
