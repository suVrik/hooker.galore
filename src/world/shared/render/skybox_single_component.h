#pragma once

#include <bgfx/bgfx.h>
#include <glm/vec4.hpp>

namespace hg {

/** `SkyboxSingleComponent` contains outline pass shaders, textures and uniforms. */
struct SkyboxSingleComponent final {
    bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle texture_irradiance = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle texture_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_irradiance_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle rotation_uniform = BGFX_INVALID_HANDLE;
};

} // namespace hg
