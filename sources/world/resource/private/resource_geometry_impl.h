#pragma once

#include "world/resource/resource_geometry.h"

namespace hg {

inline bgfx::IndexBufferHandle ResourceGeometry::get_index_buffer() const {
    assert(is_loaded());
    return *m_index_buffer;
}

inline uint32_t ResourceGeometry::get_indices_count() const {
    assert(is_loaded());
    return m_indices_count;
}

inline bgfx::VertexBufferHandle ResourceGeometry::get_vertex_buffer() const {
    assert(is_loaded());
    return *m_vertex_buffer;
}

inline uint32_t ResourceGeometry::get_vertices_count() const {
    assert(is_loaded());
    return m_vertices_count;
}

} // namespace hg
