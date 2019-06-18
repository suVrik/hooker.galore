#pragma once

#include <bgfx/bgfx.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

namespace hg {

/** `Model` is a hierarchy of nodes containing geometry primitives. */
class Model final {
public:
    /** `BasicModelVertex` is used for non-skinned meshes. */
    struct BasicModelVertex final {
        static const bgfx::VertexDecl DECLARATION;

        float x;
        float y;
        float z;
        float normal_x;
        float normal_y;
        float normal_z;
        float tangent_x;
        float tangent_y;
        float tangent_z;
        float tangent_w;
        float u;
        float v;
    };

    /** `Primitive` is a container for geometry data. All `Primitive` must be destroyed before `RenderFetchSystem`
        destructor. */
    struct Primitive final {
        Primitive() = default;
        Primitive(const Primitive& another) = delete;
        Primitive(Primitive&& another) noexcept;
        Primitive& operator=(const Primitive& another) = delete;
        Primitive& operator=(Primitive&& another) noexcept;
        ~Primitive();

        bgfx::IndexBufferHandle index_buffer   = BGFX_INVALID_HANDLE;
        bgfx::VertexBufferHandle vertex_buffer = BGFX_INVALID_HANDLE;
        size_t num_vertices = 0;
        size_t num_indices  = 0;
    };

    /** `Mesh` is a container for geometry primitives. */
    struct Mesh final {
        std::vector<Primitive> primitives;
    };

    /** `Node` is a container for other nodes. `Node` may also contain a `Mesh`. */
    struct Node final {
        std::string name;
        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;
        std::vector<Node> children;
        Mesh* mesh;
    };

    std::vector<Node> children;
};

} // namespace hg
