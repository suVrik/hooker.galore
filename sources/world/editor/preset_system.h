#pragma once

#include "core/ecs/system.h"

#include <entt/fwd.hpp>

namespace hg {

struct CameraSingleComponent;
struct HistorySingleComponent;
struct PresetSingleComponent;

/** `PresetSystem` shows UI to select preset and allows to put on the stage. */
class PresetSystem final : public NormalSystem {
public:
    explicit PresetSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    static constexpr float PLACE_PRESET_DISTANCE = 7.f;

    void show_presets_window(PresetSingleComponent& preset_single_component, HistorySingleComponent& history_single_component,
                             CameraSingleComponent& camera_single_component) const noexcept;
    void process_drag_and_drop(PresetSingleComponent& preset_single_component, HistorySingleComponent& history_single_component,
                               CameraSingleComponent& camera_single_component) const noexcept;
    entt::entity place_preset(PresetSingleComponent& preset_single_component, HistorySingleComponent& history_single_component,
                              CameraSingleComponent& camera_single_component, const std::string& preset_name, bool is_continuous) const noexcept;
};

} // namespace hg
