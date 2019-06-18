#include "core/resource/model.h"

namespace hg {

const bgfx::VertexDecl Model::BasicModelVertex::DECLARATION = []{
    bgfx::VertexDecl result;
    result.begin()
          .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
          .add(bgfx::Attrib::Normal,    3, bgfx::AttribType::Float)
          .add(bgfx::Attrib::Tangent,   3, bgfx::AttribType::Float)
          .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
          .end();
    return result;
}();

Model::Primitive::Primitive(Primitive&& another) noexcept
        : index_buffer(another.index_buffer)
        , vertex_buffer(another.vertex_buffer)
        , num_vertices(another.num_vertices)
        , num_indices(another.num_indices) {
    another.index_buffer  = BGFX_INVALID_HANDLE;
    another.vertex_buffer = BGFX_INVALID_HANDLE;
    another.num_vertices  = 0;
    another.num_indices   = 0;
}

Model::Primitive& Model::Primitive::operator=(Primitive&& another) noexcept {
    index_buffer  = another.index_buffer;
    vertex_buffer = another.vertex_buffer;
    num_vertices  = another.num_vertices;
    num_indices   = another.num_indices;

    another.index_buffer  = BGFX_INVALID_HANDLE;
    another.vertex_buffer = BGFX_INVALID_HANDLE;
    another.num_vertices  = 0;
    another.num_indices   = 0;

    return *this;
}

Model::Primitive::~Primitive() {
    if (bgfx::isValid(index_buffer)) {
        bgfx::destroy(index_buffer);
        index_buffer = BGFX_INVALID_HANDLE;
    }

    if (bgfx::isValid(vertex_buffer)) {
        bgfx::destroy(vertex_buffer);
        index_buffer = BGFX_INVALID_HANDLE;
    }
}

} // namespace hg
