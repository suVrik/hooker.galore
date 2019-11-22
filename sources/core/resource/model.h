#pragma once

#include <bgfx/bgfx.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace hg {

/** `Model` is a hierarchy of nodes containing geometry primitives. */
class Model {
public:
    /** `BasicModelVertex` is used for non-skinned meshes. */
    struct BasicModelVertex {
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
    struct Primitive {
        Primitive() = default;
        Primitive(const Primitive& another) = delete;
        Primitive(Primitive&& another);
        Primitive& operator=(const Primitive& another) = delete;
        Primitive& operator=(Primitive&& another);
        ~Primitive();

        bgfx::IndexBufferHandle index_buffer   = BGFX_INVALID_HANDLE;
        bgfx::VertexBufferHandle vertex_buffer = BGFX_INVALID_HANDLE;
        size_t num_vertices = 0;
        size_t num_indices  = 0;
    };

    /** `Mesh` is a container for geometry primitives. */
    struct Mesh {
        std::vector<Primitive> primitives;
    };

    /** `Node` is a container for other nodes. `Node` may also contain a `Mesh`. */
    struct Node {
        std::string name;
        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;
        std::vector<Node> children;
        Mesh* mesh;
    };

    /** `AABB` describes bounds of the model. */
    struct AABB {
        float min_x = std::numeric_limits<float>::max();
        float min_y = std::numeric_limits<float>::max();
        float min_z = std::numeric_limits<float>::max();
        float max_x = -std::numeric_limits<float>::max();
        float max_y = -std::numeric_limits<float>::max();
        float max_z = -std::numeric_limits<float>::max();
    };

    std::vector<Node> children;
    AABB bounds;
};

} // namespace hg
