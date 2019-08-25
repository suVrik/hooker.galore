#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `MenuSystem` shows editor menu. */
class MenuSystem final : public NormalSystem {
public:
    explicit MenuSystem(World& world) noexcept;
    void update(float elapsed_time) override;
};

} // namespace hg
