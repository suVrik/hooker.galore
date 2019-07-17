#include "core/ecs/world.h"
#include "world/editor/editor_camera_component.h"
#include "world/editor/editor_component.h"
#include "world/editor/gizmo_single_component.h"
#include "world/editor/guid_single_component.h"
#include "world/editor/history_single_component.h"
#include "world/editor/preset_single_component.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/imgui_single_component.h"
#include "world/shared/level_single_component.h"
#include "world/shared/name_single_component.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/render/aa_pass_single_component.h"
#include "world/shared/render/blockout_component.h"
#include "world/shared/render/camera_single_component.h"
#include "world/shared/render/debug_draw_pass_single_component.h"
#include "world/shared/render/geometry_pass_single_component.h"
#include "world/shared/render/hdr_pass_single_component.h"
#include "world/shared/render/light_component.h"
#include "world/shared/render/lighting_pass_single_component.h"
#include "world/shared/render/material_component.h"
#include "world/shared/render/model_component.h"
#include "world/shared/render/model_single_component.h"
#include "world/shared/render/outline_component.h"
#include "world/shared/render/outline_pass_single_component.h"
#include "world/shared/render/picking_pass_single_component.h"
#include "world/shared/render/quad_single_component.h"
#include "world/shared/render/render_single_component.h"
#include "world/shared/render/skybox_pass_single_component.h"
#include "world/shared/render/texture_single_component.h"
#include "world/shared/transform_component.h"
#include "world/shared/window_single_component.h"

#define REGISTER_COMPONENT(name) world.register_component<name>()

namespace hg {

void register_components(World& world) noexcept {
    REGISTER_COMPONENT(AAPassSingleComponent);
    REGISTER_COMPONENT(CameraSingleComponent);
    REGISTER_COMPONENT(DebugDrawPassSingleComponent);
    REGISTER_COMPONENT(GeometryPassSingleComponent);
    REGISTER_COMPONENT(GizmoSingleComponent);
    REGISTER_COMPONENT(GuidSingleComponent);
    REGISTER_COMPONENT(HistorySingleComponent);
    REGISTER_COMPONENT(HDRPassSingleComponent);
    REGISTER_COMPONENT(ImguiSingleComponent);
    REGISTER_COMPONENT(LevelSingleComponent);
    REGISTER_COMPONENT(LightingPassSingleComponent);
    REGISTER_COMPONENT(ModelSingleComponent);
    REGISTER_COMPONENT(NameSingleComponent);
    REGISTER_COMPONENT(NormalInputSingleComponent);
    REGISTER_COMPONENT(OutlinePassSingleComponent);
    REGISTER_COMPONENT(PickingPassSingleComponent);
    REGISTER_COMPONENT(PresetSingleComponent);
    REGISTER_COMPONENT(QuadSingleComponent);
    REGISTER_COMPONENT(RenderSingleComponent);
    REGISTER_COMPONENT(RunningWorldSingleComponent);
    REGISTER_COMPONENT(SelectedEntitySingleComponent);
    REGISTER_COMPONENT(SkyboxPassSingleComponent);
    REGISTER_COMPONENT(TextureSingleComponent);
    REGISTER_COMPONENT(WindowSingleComponent);

    REGISTER_COMPONENT(BlockoutComponent);
    REGISTER_COMPONENT(EditorCameraComponent);
    REGISTER_COMPONENT(EditorComponent);
    REGISTER_COMPONENT(LightComponent);
    REGISTER_COMPONENT(MaterialComponent);
    REGISTER_COMPONENT(ModelComponent);
    REGISTER_COMPONENT(OutlineComponent);
    REGISTER_COMPONENT(TransformComponent);
}

} // namespace hg

#undef REGISTER_COMPONENT
