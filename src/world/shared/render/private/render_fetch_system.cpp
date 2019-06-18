#include "core/ecs/world.h"
#include "world/shared/render/render_fetch_system.h"
#include "world/shared/window_single_component.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <SDL2/SDL_syswm.h>

namespace hg {

RenderFetchSystem::RenderFetchSystem(World& world)
        : NormalSystem(world) {
    auto& window_single_component = world.ctx<WindowSingleComponent>();

    SDL_SysWMinfo native_info;
    SDL_VERSION(&native_info.version);
    if (!SDL_GetWindowWMInfo(window_single_component.window, &native_info)) {
        throw std::runtime_error("Failed to get a native window handle!");
    }

    bgfx::PlatformData platform_data {};
#if BX_PLATFORM_WINDOWS
    platform_data.nwh = native_info.info.win.window;
#elif BX_PLATFORM_OSX
    platform_data.ndt = NULL;
    platform_data.nwh = native_info.info.cocoa.window;
#endif
    bgfx::setPlatformData(platform_data);

    if (!bgfx::init()) {
        throw std::runtime_error("Failed to initialize a renderer!");
    }

    bgfx::reset(window_single_component.width, window_single_component.height, BGFX_RESET_VSYNC);

    //bgfx::setDebug(BGFX_DEBUG_STATS);
}

RenderFetchSystem::~RenderFetchSystem() {
    bgfx::shutdown();
}

void RenderFetchSystem::update(float /*elapsed_time*/) {
    assert(world.after("WindowSystem"));

    auto& window_single_component = world.ctx<WindowSingleComponent>();
    if (window_single_component.resized) {
        bgfx::reset(window_single_component.width, window_single_component.height, BGFX_RESET_VSYNC);
    }
}

} // namespace hg
