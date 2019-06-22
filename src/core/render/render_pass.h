#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `RenderPass` enumeration specifies render pass order. */
enum RenderPass : bgfx::ViewId {
    GEOMETRY_PASS             = 0,
    LIGHTING_PASS             = 1,
    OUTLINE_PASS              = 2, // Optional
    OUTLINE_BLUR_PASS         = 3, // Optional
    DEBUG_DRAW_OFFSCREEN_PASS = 4, // Optional
    DEBUG_DRAW_ONSCREEN_PASS  = 5, // Optional
    IMGUI_PASS                = 6, // Optional
    PICKING_PASS              = 7, // Optional
    PICKING_BLIT_PASS         = 8, // Optional
};

} // namespace hg
