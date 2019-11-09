#pragma once

#include "core/ecs/system.h"

namespace hg {

class NormalInputSingleComponent;
struct CameraSingleComponent;
struct EditorCameraComponent;
struct EditorSelectionSingleComponent;
struct TransformComponent;

/** `EditorCameraSystem` updates camera in editor. */
class EditorCameraSystem final : public NormalSystem {
public:
    explicit EditorCameraSystem(World& world);
    void update(float elapsed_time) override;

private:
    static constexpr float CAMERA_SPEED                 = 5.f;
    static constexpr float CAMERA_SPEED_INCREASE_FACTOR = 3.f;
    static constexpr float CAMERA_SPEED_WHEEL_FACTOR    = 4.f;
    static constexpr float MOUSE_SENSITIVITY            = 0.004f;

    void update_attached_camera(CameraSingleComponent& camera_single_component, 
                                NormalInputSingleComponent& normal_input_single_component,
                                EditorSelectionSingleComponent& editor_selection_single_component,
                                EditorCameraComponent& editor_camera_component,
                                TransformComponent& transform_component, 
                                float speed) const;
    void update_free_camera(CameraSingleComponent& camera_single_component, 
                            NormalInputSingleComponent& normal_input_single_component,
                            EditorCameraComponent& editor_camera_component, 
                            TransformComponent& transform_component, 
                            float speed) const;
};

} // namespace hg
