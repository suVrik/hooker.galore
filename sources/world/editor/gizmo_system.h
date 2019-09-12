#pragma once

#include "core/ecs/system.h"

namespace hg {

struct CameraSingleComponent;
struct GizmoSingleComponent;
struct HistorySingleComponent;
class NormalInputSingleComponent;
struct SelectedEntitySingleComponent;

/** `GizmoSystem` shows gizmo that allows to modify transform of selected object. */
class GizmoSystem final : public NormalSystem {
public:
    explicit GizmoSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    void show_gizmo_window(GizmoSingleComponent& gizmo_single_component, NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void process_gizmo(GizmoSingleComponent& gizmo_single_component, NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void process_single_entity(CameraSingleComponent& camera_single_component, GizmoSingleComponent& gizmo_single_component, HistorySingleComponent& history_single_component,
                               NormalInputSingleComponent& normal_input_single_component, SelectedEntitySingleComponent& selected_entity_single_component) const noexcept;
    void process_multiple_entities(CameraSingleComponent& camera_single_component, GizmoSingleComponent& gizmo_single_component, HistorySingleComponent& history_single_component,
                                   NormalInputSingleComponent& normal_input_single_component, SelectedEntitySingleComponent& selected_entity_single_component) const noexcept;
};

} // namespace hg
