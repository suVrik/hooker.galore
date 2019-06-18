#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `PresetSystem` shows UI to select preset and allows to put on the stage. */
class PresetSystem final : public NormalSystem {
public:
    explicit PresetSystem(World& world) noexcept;
    void update(float elapsed_time) override;
};

} // namespace hg
