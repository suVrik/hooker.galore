#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `QuadSingleComponent` contains vertices and indices for a 2x2 quad from -1 to +1 for both axes. */
struct QuadSingleComponent final {
    static constexpr uint32_t NUM_VERTICES = 4;
    static constexpr uint32_t NUM_INDICES  = 6;

    bgfx::IndexBufferHandle index_buffer   = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle program            = BGFX_INVALID_HANDLE;
    bgfx::VertexBufferHandle vertex_buffer = BGFX_INVALID_HANDLE;
};

} // namespace hg
