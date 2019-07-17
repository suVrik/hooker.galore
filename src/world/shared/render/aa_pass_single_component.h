#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `AAPassSingleComponent` contains AA pass shaders, textures and uniforms. */
struct AAPassSingleComponent final {
    bgfx::FrameBufferHandle buffer      = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle aa_pass_program = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle color_texture   = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle pixel_size_uniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_uniform    = BGFX_INVALID_HANDLE;
};

} // namespace hg
