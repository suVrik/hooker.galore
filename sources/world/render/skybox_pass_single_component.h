#pragma once

#include "core/render/unique_handle.h"

#include <bgfx/bgfx.h>

namespace hg {

/** `SkyboxPassSingleComponent` contains skybox pass shaders, textures and uniforms. */
struct SkyboxPassSingleComponent final {
    UniqueHandle<bgfx::FrameBufferHandle> buffer;
    UniqueHandle<bgfx::ProgramHandle> program;
    UniqueHandle<bgfx::TextureHandle> color_texture;

    UniqueHandle<bgfx::UniformHandle> skybox_sampler_uniform;
    UniqueHandle<bgfx::UniformHandle> rotation_uniform;
};

} // namespace hg
