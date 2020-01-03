#pragma once

#include "core/render/unique_handle.h"

#include <glm/vec4.hpp>

namespace hg {

/** `OutlinePassSingleComponent` contains outline pass shaders, textures and uniforms. */
struct OutlinePassSingleComponent final {
    UniqueHandle<bgfx::FrameBufferHandle> buffer;

    UniqueHandle<bgfx::ProgramHandle> offscreen_program;
    UniqueHandle<bgfx::ProgramHandle> onscreen_program;

    UniqueHandle<bgfx::TextureHandle> color_texture;
    UniqueHandle<bgfx::TextureHandle> depth_texture;

    UniqueHandle<bgfx::UniformHandle> texture_sampler_uniform;
    UniqueHandle<bgfx::UniformHandle> outline_color_uniform;
    UniqueHandle<bgfx::UniformHandle> group_index_uniform;

    glm::vec4 outline_color = glm::vec4(1.f, 1.f, 0.f, 1.f);
};

} // namespace hg
