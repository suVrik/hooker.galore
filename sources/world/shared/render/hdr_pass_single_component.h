#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `HDRPassSingleComponent` contains HDR pass shaders, textures and uniforms. */
struct HDRPassSingleComponent final {
    bgfx::ProgramHandle hdr_pass_program = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_uniform  = BGFX_INVALID_HANDLE;
};

} // namespace hg
