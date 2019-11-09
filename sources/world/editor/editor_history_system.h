#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `EditorHistorySystem` shows history overlay and performs undo-redo operations. */
class EditorHistorySystem final : public NormalSystem {
public:
    explicit EditorHistorySystem(World& world);
    void update(float elapsed_time) override;
};

} // namespace hg
