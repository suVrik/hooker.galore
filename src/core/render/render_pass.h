#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `RenderPass` enumeration specifies render pass order. */
enum RenderPass : bgfx::ViewId {
    SKYBOX_PASS_RIGHT = 0,
    SKYBOX_PASS_LEFT,
    SKYBOX_PASS_TOP,
    SKYBOX_PASS_BOTTOM,
    SKYBOX_PASS_FRONT,
    SKYBOX_PASS_BACK,

    GEOMETRY_PASS = 0,
    LIGHTING_PASS,
    OUTLINE_PASS,
    OUTLINE_BLUR_PASS,
    DEBUG_DRAW_OFFSCREEN_PASS, // Optional
    DEBUG_DRAW_ONSCREEN_PASS, // Optional
    IMGUI_PASS, // Optional
    PICKING_PASS, // Optional
    PICKING_BLIT_PASS, // Optional
};

} // namespace hg
