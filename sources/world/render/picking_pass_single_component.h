#pragma once

#include "core/render/unique_handle.h"

#include <bgfx/bgfx.h>

namespace hg {

/** `PickingPassSingleComponent` contains picking pass shaders, textures and uniforms. */
struct PickingPassSingleComponent {
    UniqueHandle<bgfx::FrameBufferHandle> buffer;
    UniqueHandle<bgfx::ProgramHandle> program;

    UniqueHandle<bgfx::TextureHandle> color_texture;
    UniqueHandle<bgfx::TextureHandle> rt_color_texture;
    UniqueHandle<bgfx::TextureHandle> rt_depth_buffer;

    UniqueHandle<bgfx::UniformHandle> object_index_uniform;

    bool perform_picking = false;
    uint32_t target_frame = 0;
    std::vector<uint8_t> target_data;
};

} // namespace hg
