#pragma once

#include "core/render/unique_handle.h"
#include "world/resource/resource.h"

#include <bgfx/bgfx.h>
#include <cassert>
#include <string>

namespace hg {

class ResourceSingleComponent;

/** `ResourceGeometry` is a resource that contains render geometry. */
class ResourceGeometry : public Resource {
public:
    /** Load geometry from specified `path`. */
    bool load(ResourceSingleComponent& resource_single_component, const std::string& path);

    /** Return index buffer. */
    bgfx::IndexBufferHandle get_index_buffer() const;

    /** Return the number of indices in index buffer. */
    uint32_t get_indices_count() const;

    /** Return vertex buffer. */
    bgfx::VertexBufferHandle get_vertex_buffer() const;

    /** Return the number of vertices in vertex buffer. */
    uint32_t get_vertices_count() const;

private:
    UniqueHandle<bgfx::IndexBufferHandle> m_index_buffer;
    UniqueHandle<bgfx::VertexBufferHandle> m_vertex_buffer;
    uint32_t m_indices_count = 0;
    uint32_t m_vertices_count = 0;
};

} // namespace hg

#include "world/resource/private/resource_geometry_impl.h"
