#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `GridSystem` draws a grid in editor. */
class GridSystem final : public NormalSystem {
public:
    explicit GridSystem(World& world) noexcept;
    void update(float elapsed_time) override;
};

} // namespace hg
