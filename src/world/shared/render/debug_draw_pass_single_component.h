#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `DebugDrawSingleComponent` holds programs and uniforms needed for debug draw pass. */
struct DebugDrawPassSingleComponent final {
    bgfx::TextureHandle color_texture = BGFX_INVALID_HANDLE;
    bgfx::FrameBufferHandle buffer    = BGFX_INVALID_HANDLE;

    bgfx::ProgramHandle solid_program    = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle textured_program = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_uniform  = BGFX_INVALID_HANDLE;
};

} // namespace hg
