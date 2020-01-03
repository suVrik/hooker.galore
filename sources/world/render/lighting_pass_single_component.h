#pragma once

#include "core/render/unique_handle.h"

namespace hg {

/** `LightingPassSingleComponent` contains lighting pass shaders, textures and uniforms. */
struct LightingPassSingleComponent final {
    UniqueHandle<bgfx::FrameBufferHandle> buffer;
    UniqueHandle<bgfx::ProgramHandle> program;
    UniqueHandle<bgfx::TextureHandle> color_texture;

    UniqueHandle<bgfx::UniformHandle> color_roughness_sampler_uniform;
    UniqueHandle<bgfx::UniformHandle> normal_metal_ao_sampler_uniform;
    UniqueHandle<bgfx::UniformHandle> depth_sampler_uniform;

    UniqueHandle<bgfx::UniformHandle> skybox_irradiance_sampler_uniform;
    UniqueHandle<bgfx::UniformHandle> skybox_prefilter_sampler_uniform;
    UniqueHandle<bgfx::UniformHandle> skybox_lut_sampler_uniform;

    UniqueHandle<bgfx::UniformHandle> light_color_uniform;
    UniqueHandle<bgfx::UniformHandle> light_position_uniform;
    UniqueHandle<bgfx::UniformHandle> skybox_mip_prefilter_max_uniform;
};

} // namespace hg
