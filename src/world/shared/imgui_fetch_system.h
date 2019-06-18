#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `ImguiFetchSystem` fetches input data from OS to ImGui. */
class ImguiFetchSystem final : public NormalSystem {
public:
    explicit ImguiFetchSystem(World& world) noexcept;
    ~ImguiFetchSystem() override;
    void update(float elapsed_time) override;
};

} // namespace hg
