#include "core/ecs/world.h"
#include "world/editor/editor_camera_system.h"
#include "world/editor/editor_file_system.h"
#include "world/editor/editor_gizmo_system.h"
#include "world/editor/editor_grid_system.h"
#include "world/editor/editor_history_system.h"
#include "world/editor/editor_menu_system.h"
#include "world/editor/editor_preset_system.h"
#include "world/editor/editor_property_editor_system.h"
#include "world/editor/editor_selection_system.h"
#include "world/shared/imgui_fetch_system.h"
#include "world/shared/physics/physics_character_controller_system.h"
#include "world/shared/physics/physics_fetch_system.h"
#include "world/shared/physics/physics_initialization_system.h"
#include "world/shared/physics/physics_rigid_body_system.h"
#include "world/shared/physics/physics_shape_system.h"
#include "world/shared/physics/physics_simulate_system.h"
#include "world/shared/render/aa_pass_system.h"
#include "world/shared/render/camera_system.h"
#include "world/shared/render/debug_draw_pass_system.h"
#include "world/shared/render/geometry_pass_system.h"
#include "world/shared/render/hdr_pass_system.h"
#include "world/shared/render/imgui_pass_system.h"
#include "world/shared/render/lighting_pass_system.h"
#include "world/shared/render/outline_pass_system.h"
#include "world/shared/render/picking_pass_system.h"
#include "world/shared/render/quad_system.h"
#include "world/shared/render/render_fetch_system.h"
#include "world/shared/render/render_system.h"
#include "world/shared/render/skybox_pass_system.h"
#include "world/shared/resource_system.h"
#include "world/shared/window_system.h"

#define REGISTER_SYSTEM(name) world.register_system<name>(#name)

namespace hg {

void register_systems(World& world) noexcept {
    REGISTER_SYSTEM(AAPassSystem);
    REGISTER_SYSTEM(CameraSystem);
    REGISTER_SYSTEM(DebugDrawPassSystem);
    REGISTER_SYSTEM(EditorCameraSystem);
    REGISTER_SYSTEM(EditorFileSystem);
    REGISTER_SYSTEM(EditorGizmoSystem);
    REGISTER_SYSTEM(EditorGridSystem);
    REGISTER_SYSTEM(EditorHistorySystem);
    REGISTER_SYSTEM(EditorMenuSystem);
    REGISTER_SYSTEM(EditorPresetSystem);
    REGISTER_SYSTEM(EditorPropertyEditorSystem);
    REGISTER_SYSTEM(EditorSelectionSystem);
    REGISTER_SYSTEM(GeometryPassSystem);
    REGISTER_SYSTEM(HDRPassSystem);
    REGISTER_SYSTEM(ImguiFetchSystem);
    REGISTER_SYSTEM(ImguiPassSystem);
    REGISTER_SYSTEM(LightingPassSystem);
    REGISTER_SYSTEM(OutlinePassSystem);
    REGISTER_SYSTEM(PhysicsCharacterControllerSystem);
    REGISTER_SYSTEM(PhysicsFetchSystem);
    REGISTER_SYSTEM(PhysicsInitializationSystem);
    REGISTER_SYSTEM(PhysicsRigidBodySystem);
    REGISTER_SYSTEM(PhysicsShapeSystem);
    REGISTER_SYSTEM(PhysicsSimulateSystem);
    REGISTER_SYSTEM(PickingPassSystem);
    REGISTER_SYSTEM(QuadSystem);
    REGISTER_SYSTEM(RenderFetchSystem);
    REGISTER_SYSTEM(RenderSystem);
    REGISTER_SYSTEM(ResourceSystem);
    REGISTER_SYSTEM(SkyboxPassSystem);
    REGISTER_SYSTEM(WindowSystem);
}

} // namespace hg

#undef REGISTER_SYSTEM
