#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `SkyboxPassSingleComponent` contains skybox pass shaders, textures and uniforms. */
struct SkyboxPassSingleComponent final {
    bgfx::FrameBufferHandle buffer          = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle skybox_pass_program = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle color_texture       = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle depth_uniform          = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle rotation_uniform       = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle skybox_texture_uniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_uniform        = BGFX_INVALID_HANDLE;
};

} // namespace hg
