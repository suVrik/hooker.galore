#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `GeometryPassSingleComponent` contains geometry pass shaders, uniforms and framebuffers. */
struct GeometryPassSingleComponent final {
    bgfx::TextureHandle color_roughness_texture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle normal_metal_ao_texture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle depth_texture           = BGFX_INVALID_HANDLE;
    bgfx::FrameBufferHandle gbuffer             = BGFX_INVALID_HANDLE;

    bgfx::ProgramHandle geometry_pass_program             = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle geometry_no_parallax_pass_program = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle geometry_blockout_pass_program    = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle color_roughness_uniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle normal_metal_ao_uniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle parallax_uniform        = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle parallax_scale_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle norm_matrix_uniform     = BGFX_INVALID_HANDLE;
};

} // namespace hg
