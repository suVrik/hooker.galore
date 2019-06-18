#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `RenderPass` enumeration specifies render pass order. */
enum RenderPass : bgfx::ViewId {
    GEOMETRY_PASS             = 0,
    LIGHTING_PASS             = 1,
    DEBUG_DRAW_OFFSCREEN_PASS = 2, // Optional
    DEBUG_DRAW_ONSCREEN_PASS  = 3, // Optional
    IMGUI_PASS                = 4, // Optional
};

} // namespace hg
