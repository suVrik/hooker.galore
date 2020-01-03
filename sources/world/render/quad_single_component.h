#pragma once

#include "core/render/unique_handle.h"

namespace hg {

/** `QuadSingleComponent` contains vertices and indices for a 2x2 quad from -1 to +1 for both axes. */
struct QuadSingleComponent final {
    static constexpr uint32_t NUM_VERTICES = 4;
    static constexpr uint32_t NUM_INDICES  = 6;

    UniqueHandle<bgfx::IndexBufferHandle> index_buffer;
    UniqueHandle<bgfx::VertexBufferHandle> vertex_buffer;
    UniqueHandle<bgfx::ProgramHandle> program;
    UniqueHandle<bgfx::UniformHandle> texture_sampler_uniform;
};

} // namespace hg
