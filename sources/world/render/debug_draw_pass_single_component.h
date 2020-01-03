#pragma once

#include "core/render/unique_handle.h"

namespace hg {

/** `DebugDrawSingleComponent` holds programs and uniforms needed for debug draw pass. */
struct DebugDrawPassSingleComponent final {
    UniqueHandle<bgfx::FrameBufferHandle> buffer;
    UniqueHandle<bgfx::ProgramHandle> solid_program;
    UniqueHandle<bgfx::ProgramHandle> textured_program;
    UniqueHandle<bgfx::TextureHandle> color_texture;
    UniqueHandle<bgfx::UniformHandle> texture_sampler_uniform;
};

} // namespace hg
