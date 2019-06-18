#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `ImguiRenderSystem` displays ImGui on the screen. */
class ImguiRenderSystem final : public NormalSystem {
public:
    explicit ImguiRenderSystem(World& world) noexcept;
    ~ImguiRenderSystem() override;
    void update(float elapsed_time) override;
};

} // namespace hg
