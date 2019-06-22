#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `PickingPassSingleComponent` contains picking pass shaders, textures and uniforms. */
struct PickingPassSingleComponent final {
    bgfx::ProgramHandle program              = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle object_index_uniform = BGFX_INVALID_HANDLE;

    bgfx::TextureHandle color_texture        = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle rt_color_texture     = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle rt_depth_buffer      = BGFX_INVALID_HANDLE;
    bgfx::FrameBufferHandle buffer           = BGFX_INVALID_HANDLE;

    bool perform_picking = false;
    uint32_t target_frame = 0;
    std::vector<uint8_t> target_data;
};

} // namespace hg
