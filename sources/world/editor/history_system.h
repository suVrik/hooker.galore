#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `HistorySystem` shows history overlay and performs undo-redo operations. */
class HistorySystem final : public NormalSystem {
public:
    explicit HistorySystem(World& world) noexcept;
    void update(float elapsed_time) override;
};

} // namespace hg
