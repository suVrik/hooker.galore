#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `LightingPassSingleComponent` contains lighting pass shaders, textures and uniforms. */
struct LightingPassSingleComponent final {
    bgfx::FrameBufferHandle buffer                  = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle lighting_pass_program       = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle lighting_pass_point_program = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle color_texture               = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle color_roughness_uniform     = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle depth_uniform               = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle dir_light_position_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle dir_light_view_proj_uniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle light_color_uniform         = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle light_position_uniform      = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle normal_metal_ao_uniform     = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle shadow_map_texture_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_uniform             = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle skybox_mip_prefilter_max_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle skybox_texture_irradiance_uniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle skybox_texture_lut_uniform        = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle skybox_texture_prefilter_uniform  = BGFX_INVALID_HANDLE;
};

} // namespace hg
