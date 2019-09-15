#pragma once

#include "core/ecs/system.h"

namespace hg {

class NormalInputSingleComponent;
struct EditorSelectionSingleComponent;

/** `EditorSelectionSystem` shows Entity list UI, allows to pick an entity and stores it in
    `EditorSelectionSingleComponent`. */
class EditorSelectionSystem final : public NormalSystem {
public:
    explicit EditorSelectionSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    void show_level_window(EditorSelectionSingleComponent& editor_selection_single_component, 
                           NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void perform_picking(EditorSelectionSingleComponent& editor_selection_single_component,
                         NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void delete_selected(EditorSelectionSingleComponent& editor_selection_single_component,
                         NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void select_all(EditorSelectionSingleComponent& editor_selection_single_component, 
                    NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void clear_selection(EditorSelectionSingleComponent& editor_selection_single_component, 
                         NormalInputSingleComponent& normal_input_single_component) const noexcept;
};

} // namespace hg
