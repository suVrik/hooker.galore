#pragma once

#include "core/ecs/system.h"

namespace hg {

class NormalInputSingleComponent;
struct CameraSingleComponent;
struct EditorGizmoSingleComponent;
struct EditorHistorySingleComponent;
struct EditorSelectionSingleComponent;

/** `EditorGizmoSystem` shows gizmo that allows to modify transform of selected object. */
class EditorGizmoSystem final : public NormalSystem {
public:
    explicit EditorGizmoSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    void show_gizmo_window(EditorGizmoSingleComponent& editor_gizmo_single_component, 
                           NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void process_gizmo(EditorGizmoSingleComponent& editor_gizmo_single_component, 
                       NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void process_single_entity(CameraSingleComponent& camera_single_component, 
                               EditorGizmoSingleComponent& editor_gizmo_single_component, 
                               EditorHistorySingleComponent& editor_history_single_component,
                               NormalInputSingleComponent& normal_input_single_component,
                               EditorSelectionSingleComponent& editor_selection_single_component) const noexcept;
    void process_multiple_entities(CameraSingleComponent& camera_single_component,
                                   EditorGizmoSingleComponent& editor_gizmo_single_component,
                                   EditorHistorySingleComponent& editor_history_single_component,
                                   NormalInputSingleComponent& normal_input_single_component,
                                   EditorSelectionSingleComponent& editor_selection_single_component) const noexcept;
};

} // namespace hg
