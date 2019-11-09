#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `EditorMenuSystem` shows editor menu. */
class EditorMenuSystem final : public NormalSystem {
public:
    explicit EditorMenuSystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
