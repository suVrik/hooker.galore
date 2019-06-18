#include "core/ecs/world.h"
#include "world/editor/editor_camera_system.h"
#include "world/editor/editor_grid_system.h"
#include "world/editor/preset_system.h"
#include "world/shared/imgui_fetch_system.h"
#include "world/shared/imgui_render_system.h"
#include "world/shared/render/camera_system.h"
#include "world/shared/render/debug_draw_pass_system.h"
#include "world/shared/render/geometry_pass_system.h"
#include "world/shared/render/lighting_pass_system.h"
#include "world/shared/render/quad_system.h"
#include "world/shared/render/render_fetch_system.h"
#include "world/shared/render/render_system.h"
#include "world/shared/resource_system.h"
#include "world/shared/window_system.h"

#define REGISTER_SYSTEM(name) world.register_system<name>(#name)

namespace hg {

void register_systems(World& world) noexcept {
    REGISTER_SYSTEM(CameraSystem);
    REGISTER_SYSTEM(DebugDrawPassSystem);
    REGISTER_SYSTEM(EditorCameraSystem);
    REGISTER_SYSTEM(EditorGridSystem);
    REGISTER_SYSTEM(GeometryPassSystem);
    REGISTER_SYSTEM(ImguiFetchSystem);
    REGISTER_SYSTEM(ImguiRenderSystem);
    REGISTER_SYSTEM(LightingPassSystem);
    REGISTER_SYSTEM(PresetSystem);
    REGISTER_SYSTEM(QuadSystem);
    REGISTER_SYSTEM(RenderFetchSystem);
    REGISTER_SYSTEM(RenderSystem);
    REGISTER_SYSTEM(ResourceSystem);
    REGISTER_SYSTEM(WindowSystem);
}

} // namespace hg

#undef REGISTER_SYSTEM
