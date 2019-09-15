#pragma once

#include "core/ecs/system.h"

#include <entt/fwd.hpp>
#include <string>

namespace hg {

struct CameraSingleComponent;
struct EditorHistorySingleComponent;
struct EditorPresetSingleComponent;

/** `EditorPresetSystem` shows UI to select preset and allows to put on the stage. */
class EditorPresetSystem final : public NormalSystem {
public:
    explicit EditorPresetSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    static constexpr float PLACE_PRESET_DISTANCE = 7.f;

    void show_presets_window(EditorPresetSingleComponent& editor_preset_single_component,
                             EditorHistorySingleComponent& editor_history_single_component,
                             CameraSingleComponent& camera_single_component) const noexcept;
    void process_drag_and_drop(EditorPresetSingleComponent& editor_preset_single_component,
                               EditorHistorySingleComponent& editor_history_single_component,
                               CameraSingleComponent& camera_single_component) const noexcept;
    entt::entity place_preset(EditorPresetSingleComponent& editor_preset_single_component,
                              EditorHistorySingleComponent& editor_history_single_component,
                              CameraSingleComponent& camera_single_component,
                              const std::string& preset_name, bool is_continuous) const noexcept;
};

} // namespace hg
