#pragma once

#include "core/render/unique_handle.h"

namespace hg {

/** `AAPassSingleComponent` contains AA pass shaders, textures and uniforms. */
struct AAPassSingleComponent final {
    UniqueHandle<bgfx::FrameBufferHandle> buffer;
    UniqueHandle<bgfx::ProgramHandle> program;
    UniqueHandle<bgfx::TextureHandle> color_texture;
    UniqueHandle<bgfx::UniformHandle> color_sampler_uniform;
};

} // namespace hg
