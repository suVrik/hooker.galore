#pragma once

#include <bgfx/bgfx.h>
#include <glm/vec4.hpp>

namespace hg {

/** `OutlinePassSingleComponent` contains outline pass shaders, textures and uniforms. */
struct OutlinePassSingleComponent final {
    bgfx::FrameBufferHandle buffer = BGFX_INVALID_HANDLE;

    bgfx::ProgramHandle outline_blur_pass_program = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle outline_pass_program      = BGFX_INVALID_HANDLE;

    bgfx::TextureHandle color_texture = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle depth_texture = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle outline_color_uniform = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle texture_uniform       = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle group_index_uniform   = BGFX_INVALID_HANDLE;

    glm::vec4 outline_color = glm::vec4(1.f, 1.f, 0.f, 1.f);
};

} // namespace hg
