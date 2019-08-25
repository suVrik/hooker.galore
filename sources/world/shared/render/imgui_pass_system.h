#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `ImguiPassSystem` displays ImGui on the screen. */
class ImguiPassSystem final : public NormalSystem {
public:
    explicit ImguiPassSystem(World& world) noexcept;
    ~ImguiPassSystem() override;
    void update(float elapsed_time) override;
};

} // namespace hg
