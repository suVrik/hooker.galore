#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `RenderPass` enumeration specifies render pass order. */
enum RenderPass : bgfx::ViewId {
    GEOMETRY_PASS = 0,
    LIGHTING_PASS,
    SKYBOX_PASS,
    AA_PASS,
    HDR_PASS,
    OUTLINE_PASS,
    OUTLINE_BLUR_PASS,
    DEBUG_DRAW_OFFSCREEN_PASS,
    DEBUG_DRAW_ONSCREEN_PASS,
    IMGUI_PASS,
    PICKING_PASS,
    PICKING_BLIT_PASS,
};

} // namespace hg
