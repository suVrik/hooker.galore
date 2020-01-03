#pragma once

#include "core/render/unique_handle.h"

namespace hg {

/** `GeometryPassSingleComponent` contains geometry pass shaders, uniforms and frame buffers. */
struct GeometryPassSingleComponent final {
    UniqueHandle<bgfx::FrameBufferHandle> buffer;

    UniqueHandle<bgfx::ProgramHandle> program;

    UniqueHandle<bgfx::TextureHandle> color_roughness_texture;
    UniqueHandle<bgfx::TextureHandle> depth_texture;
    UniqueHandle<bgfx::TextureHandle> depth_stencil_texture;
    UniqueHandle<bgfx::TextureHandle> normal_metal_ao_texture;

    UniqueHandle<bgfx::UniformHandle> color_roughness_sampler_uniform;
    UniqueHandle<bgfx::UniformHandle> normal_metal_ao_sampler_uniform;
};

} // namespace hg
