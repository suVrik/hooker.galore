#pragma once

#include <bgfx/bgfx.h>
#include <glm/vec4.hpp>

namespace hg {

/** `SkyboxSingleComponent` contains outline pass shaders, textures and uniforms. */
struct SkyboxSingleComponent final {
    uint16_t side_size = 0;
    float mip_prefilter_max = 0;
    bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle texture_irradiance = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle texture_prefilter = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle texture_lut = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle texture_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_irradiance_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_prefilter_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_lut_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle mip_prefilter_max_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle rotation_uniform = BGFX_INVALID_HANDLE;
};

} // namespace hg
