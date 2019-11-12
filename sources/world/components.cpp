#include "core/ecs/component_manager.h"
#include "core/ecs/world.h"
#include "world/editor/editor_camera_component.h"
#include "world/editor/editor_file_single_component.h"
#include "world/editor/editor_gizmo_single_component.h"
#include "world/editor/editor_history_single_component.h"
#include "world/editor/editor_preset_single_component.h"
#include "world/editor/editor_selection_single_component.h"
#include "world/physics/physics_box_shape_component.h"
#include "world/physics/physics_box_shape_private_component.h"
#include "world/physics/physics_character_controller_component.h"
#include "world/physics/physics_character_controller_private_component.h"
#include "world/physics/physics_character_controller_single_component.h"
#include "world/physics/physics_single_component.h"
#include "world/physics/physics_static_rigid_body_component.h"
#include "world/physics/physics_static_rigid_body_private_component.h"
#include "world/render/aa_pass_single_component.h"
#include "world/render/blockout_component.h"
#include "world/render/camera_single_component.h"
#include "world/render/debug_draw_pass_single_component.h"
#include "world/render/geometry_pass_single_component.h"
#include "world/render/hdr_pass_single_component.h"
#include "world/render/light_component.h"
#include "world/render/lighting_pass_single_component.h"
#include "world/render/material_component.h"
#include "world/render/model_component.h"
#include "world/render/model_single_component.h"
#include "world/render/outline_component.h"
#include "world/render/outline_pass_single_component.h"
#include "world/render/picking_pass_single_component.h"
#include "world/render/quad_single_component.h"
#include "world/render/render_single_component.h"
#include "world/render/skybox_pass_single_component.h"
#include "world/render/texture_single_component.h"
#include "world/shared/imgui_single_component.h"
#include "world/shared/level_single_component.h"
#include "world/shared/name_component.h"
#include "world/shared/name_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#define REGISTER_COMPONENT(name) ComponentManager::register_component<name>()

namespace hg {

void register_components() {
    REGISTER_COMPONENT(AAPassSingleComponent);
    REGISTER_COMPONENT(CameraSingleComponent);
    REGISTER_COMPONENT(DebugDrawPassSingleComponent);
    REGISTER_COMPONENT(EditorFileSingleComponent);
    REGISTER_COMPONENT(EditorGizmoSingleComponent);
    REGISTER_COMPONENT(EditorHistorySingleComponent);
    REGISTER_COMPONENT(EditorPresetSingleComponent);
    REGISTER_COMPONENT(EditorSelectionSingleComponent);
    REGISTER_COMPONENT(GeometryPassSingleComponent);
    REGISTER_COMPONENT(HDRPassSingleComponent);
    REGISTER_COMPONENT(ImguiSingleComponent);
    REGISTER_COMPONENT(LevelSingleComponent);
    REGISTER_COMPONENT(LightingPassSingleComponent);
    REGISTER_COMPONENT(ModelSingleComponent);
    REGISTER_COMPONENT(NameSingleComponent);
    REGISTER_COMPONENT(NormalInputSingleComponent);
    REGISTER_COMPONENT(OutlinePassSingleComponent);
    REGISTER_COMPONENT(PhysicsCharacterControllerSingleComponent);
    REGISTER_COMPONENT(PhysicsSingleComponent);
    REGISTER_COMPONENT(PickingPassSingleComponent);
    REGISTER_COMPONENT(QuadSingleComponent);
    REGISTER_COMPONENT(RenderSingleComponent);
    REGISTER_COMPONENT(RunningWorldSingleComponent);
    REGISTER_COMPONENT(SkyboxPassSingleComponent);
    REGISTER_COMPONENT(TextureSingleComponent);
    REGISTER_COMPONENT(WindowSingleComponent);

    REGISTER_COMPONENT(BlockoutComponent);
    REGISTER_COMPONENT(EditorCameraComponent);
    REGISTER_COMPONENT(LightComponent);
    REGISTER_COMPONENT(MaterialComponent);
    REGISTER_COMPONENT(ModelComponent);
    REGISTER_COMPONENT(NameComponent);
    REGISTER_COMPONENT(OutlineComponent);
    REGISTER_COMPONENT(PhysicsBoxShapeComponent);
    REGISTER_COMPONENT(PhysicsBoxShapePrivateComponent);
    REGISTER_COMPONENT(PhysicsCharacterControllerComponent);
    REGISTER_COMPONENT(PhysicsCharacterControllerPrivateComponent);
    REGISTER_COMPONENT(PhysicsStaticRigidBodyComponent);
    REGISTER_COMPONENT(PhysicsStaticRigidBodyPrivateComponent);
    REGISTER_COMPONENT(TransformComponent);
}

} // namespace hg

#undef REGISTER_COMPONENT
