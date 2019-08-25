#include "core/ecs/world.h"
#include "world/editor/editor_camera_system.h"
#include "world/editor/editor_file_system.h"
#include "world/editor/entity_selection_system.h"
#include "world/editor/gizmo_system.h"
#include "world/editor/grid_system.h"
#include "world/editor/history_system.h"
#include "world/editor/menu_system.h"
#include "world/editor/preset_system.h"
#include "world/editor/property_editor_system.h"
#include "world/shared/imgui_fetch_system.h"
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
    REGISTER_SYSTEM(EntitySelectionSystem);
    REGISTER_SYSTEM(GeometryPassSystem);
    REGISTER_SYSTEM(GizmoSystem);
    REGISTER_SYSTEM(GridSystem);
    REGISTER_SYSTEM(HDRPassSystem);
    REGISTER_SYSTEM(HistorySystem);
    REGISTER_SYSTEM(ImguiFetchSystem);
    REGISTER_SYSTEM(ImguiPassSystem);
    REGISTER_SYSTEM(LightingPassSystem);
    REGISTER_SYSTEM(MenuSystem);
    REGISTER_SYSTEM(OutlinePassSystem);
    REGISTER_SYSTEM(PickingPassSystem);
    REGISTER_SYSTEM(PresetSystem);
    REGISTER_SYSTEM(PropertyEditorSystem);
    REGISTER_SYSTEM(QuadSystem);
    REGISTER_SYSTEM(RenderFetchSystem);
    REGISTER_SYSTEM(RenderSystem);
    REGISTER_SYSTEM(ResourceSystem);
    REGISTER_SYSTEM(SkyboxPassSystem);
    REGISTER_SYSTEM(WindowSystem);
}

} // namespace hg

#undef REGISTER_SYSTEM