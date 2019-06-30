#pragma once

#include <bgfx/bgfx.h>
#include <glm/vec4.hpp>

namespace hg {

/** `SkyboxSingleComponent` contains skybox textures. */
struct SkyboxSingleComponent final {
    uint16_t side_size = 0;
    float mip_prefilter_max = 0;
    bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle texture_irradiance = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle texture_prefilter = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle texture_lut = BGFX_INVALID_HANDLE;
};

} // namespace hg
