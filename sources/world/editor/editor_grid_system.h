#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `EditorGridSystem` draws a grid in editor. */
class EditorGridSystem final : public NormalSystem {
public:
    explicit EditorGridSystem(World& world) noexcept;
    void update(float elapsed_time) override;
};

} // namespace hg
