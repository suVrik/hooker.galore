#pragma once

#include "core/ecs/system.h"

namespace hg {

struct CameraSingleComponent;
struct EditorCameraComponent;
class NormalInputSingleComponent;
struct SelectedEntitySingleComponent;
struct TransformComponent;

/** `EditorCameraSystem` updates camera in editor. */
class EditorCameraSystem final : public NormalSystem {
public:
    explicit EditorCameraSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    void update_attached_camera(CameraSingleComponent& camera_single_component, NormalInputSingleComponent& normal_input_single_component,
                                SelectedEntitySingleComponent& selected_entity_single_component, EditorCameraComponent& editor_camera_component,
                                TransformComponent& transform_component, float speed) const noexcept;
    void update_free_camera(CameraSingleComponent& camera_single_component, NormalInputSingleComponent& normal_input_single_component,
                            EditorCameraComponent& editor_camera_component, TransformComponent& transform_component, float speed) const noexcept;
};

} // namespace hg
