#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `LightingPassSingleComponent` contains lighting pass shaders, textures and uniforms. */
struct LightingPassSingleComponent final {
    bgfx::ProgramHandle lighting_pass_program   = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle color_roughness_uniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle normal_metal_ao_uniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle depth_uniform           = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle light_position_uniform  = BGFX_INVALID_HANDLE;
};

} // namespace hg
