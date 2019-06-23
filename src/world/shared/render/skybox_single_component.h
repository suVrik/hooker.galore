#pragma once

#include <bgfx/bgfx.h>
#include <glm/vec4.hpp>

namespace hg {

/** `SkyboxSingleComponent` contains outline pass shaders, textures and uniforms. */
struct SkyboxSingleComponent final {
    uint16_t side = 0;
    bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle texture_uniform  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle rotation_uniform = BGFX_INVALID_HANDLE;
};

} // namespace hg
