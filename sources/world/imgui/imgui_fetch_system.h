#pragma once

#include "core/ecs/system.h"

namespace hg {

/** `ImguiFetchSystem` fetches input data from OS to ImGui. */
class ImguiFetchSystem final : public NormalSystem {
public:
    explicit ImguiFetchSystem(World& world);
    ~ImguiFetchSystem() override;
    void update(float elapsed_time) override;

private:
    void update_imgui(float elapsed_time) const;
    void build_dock_space() const;
};

} // namespace hg
