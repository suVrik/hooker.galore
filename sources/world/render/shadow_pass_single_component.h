#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `ShadowPassSingleComponent` contains shadow pass shaders, uniforms and framebuffers. */
struct ShadowPassSingleComponent final {
    bgfx::FrameBufferHandle gbuffer = BGFX_INVALID_HANDLE;

    bgfx::ProgramHandle shadow_pass_program = BGFX_INVALID_HANDLE;

    bgfx::TextureHandle shadow_map_texture = BGFX_INVALID_HANDLE;

    bgfx::TextureHandle depth_stencil_texture   = BGFX_INVALID_HANDLE;
};

} // namespace hg
