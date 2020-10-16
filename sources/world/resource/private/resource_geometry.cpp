#include "world/resource/resource_geometry.h"
#include "world/resource/resource_single_component.h"

#define TINYGLTF_NO_STB_IMAGE       1
#define TINYGLTF_NO_STB_IMAGE_WRITE 1

#include <bx/debug.h>
#include <fmt/format.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <tiny_gltf.h>

namespace hg {

namespace resource_geometry_details {

struct Vertex {
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

const bgfx::VertexDecl Vertex::DECLARATION = [] {
    bgfx::VertexDecl result;
    result.begin()
        .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal,    3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Tangent,   4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();
    return result;
}();

void load_gltf_primitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const glm::mat4& transform, std::vector<uint16_t>& index_buffer, std::vector<Vertex>& vertex_buffer) {
    size_t vertex_offset = vertex_buffer.size();
    size_t vertices_count = 0;
    size_t attributes = 0;

    for (const auto& [attribute, accessor_index] : primitive.attributes) {
        if (accessor_index < 0 || accessor_index >= model.accessors.size()) {
            throw std::runtime_error("Invalid property accessor.");
        }

        const tinygltf::Accessor& accessor = model.accessors[accessor_index];
        if (accessor.sparse.isSparse ||
            accessor.count == 0 ||
            accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size()) {
            throw std::runtime_error("Invalid property accessor.");
        }

        const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
        if (buffer_view.target != TINYGLTF_TARGET_ARRAY_BUFFER ||
            buffer_view.buffer < 0 || buffer_view.buffer >= model.buffers.size()) {
            throw std::runtime_error("Invalid property accessor.");
        }

        const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
        if (accessor.byteOffset + buffer_view.byteOffset + buffer_view.byteLength > buffer.data.size() ||
            buffer_view.byteStride * (accessor.count - 1) >= buffer_view.byteLength) {
            throw std::runtime_error("Invalid property accessor.");
        }

        const uint8_t* buffer_data = buffer.data.data() + accessor.byteOffset + buffer_view.byteOffset;

        if (vertices_count == 0) {
            vertices_count = accessor.count;
            vertex_buffer.resize(vertex_offset + vertices_count);
        }

        if (attribute == "POSITION") {
            size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(float) * 3 : buffer_view.byteStride;

            if (accessor.type != TINYGLTF_TYPE_VEC3 ||
                accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
                accessor.count != vertices_count ||
                byte_stride < sizeof(float) * 3 ||
                byte_stride * (vertices_count - 1) + sizeof(float) * 3 > buffer_view.byteLength ||
                (attributes & 1) != 0) {
                throw std::runtime_error("Invalid POSITION accessor.");
            }
            attributes |= 1;

            for (size_t i = 0; i < vertices_count; i++) {
                const auto* position_data = reinterpret_cast<const float*>(buffer_data + i * byte_stride);

                glm::vec4 position(position_data[0], position_data[1], -position_data[2], 1.f);
                glm::vec4 transformed_position(transform * position);

                vertex_buffer[vertex_offset + i].x = transformed_position.x;
                vertex_buffer[vertex_offset + i].y = transformed_position.y;
                vertex_buffer[vertex_offset + i].z = transformed_position.z;
            }
        } else if (attribute == "NORMAL") {
            size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(float) * 3 : buffer_view.byteStride;

            if (accessor.type != TINYGLTF_TYPE_VEC3 ||
                accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
                accessor.count != vertices_count ||
                byte_stride < sizeof(float) * 3 ||
                byte_stride * (vertices_count - 1) + sizeof(float) * 3 > buffer_view.byteLength ||
                (attributes & 2) != 0) {
                throw std::runtime_error("Invalid NORMAL accessor.");
            }
            attributes |= 2;

            for (size_t i = 0; i < vertices_count; i++) {
                const auto* normal_data = reinterpret_cast<const float*>(buffer_data + i * byte_stride);
                vertex_buffer[vertex_offset + i].normal_x = normal_data[0];
                vertex_buffer[vertex_offset + i].normal_y = normal_data[1];
                vertex_buffer[vertex_offset + i].normal_z = -normal_data[2];
            }
        } else if (attribute == "TANGENT") {
            size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(float) * 4 : buffer_view.byteStride;

            if (accessor.type != TINYGLTF_TYPE_VEC4 ||
                accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
                accessor.count != vertices_count ||
                byte_stride < sizeof(float) * 3 ||
                byte_stride * (vertices_count - 1) + sizeof(float) * 4 > buffer_view.byteLength ||
                (attributes & 4) != 0) {
                throw std::runtime_error("Invalid TANGENT accessor.");
            }
            attributes |= 4;

            for (size_t i = 0; i < vertices_count; i++) {
                const auto* tangent_data = reinterpret_cast<const float*>(buffer_data + i * byte_stride);
                vertex_buffer[vertex_offset + i].tangent_x = -tangent_data[0];
                vertex_buffer[vertex_offset + i].tangent_y = tangent_data[1];
                vertex_buffer[vertex_offset + i].tangent_z = tangent_data[2];
                vertex_buffer[vertex_offset + i].tangent_w = tangent_data[3];
            }
        } else if (attribute == "TEXCOORD_0") {
            if (accessor.type != TINYGLTF_TYPE_VEC2 ||
                accessor.count != vertices_count ||
                (attributes & 8) != 0) {
                throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
            }
            attributes |= 8;

            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(float) * 2 : buffer_view.byteStride;
                if (byte_stride >= sizeof(float) * 2 && byte_stride * (vertices_count - 1) + sizeof(float) * 2 <= buffer_view.byteLength) {
                    for (size_t i = 0; i < vertices_count; i++) {
                        const auto* texcoord_data = reinterpret_cast<const float*>(buffer_data + i * byte_stride);
                        vertex_buffer[vertex_offset + i].u = texcoord_data[0];
                        vertex_buffer[vertex_offset + i].v = texcoord_data[1];
                    }
                } else {
                    throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
                }
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(uint8_t) * 2 : buffer_view.byteStride;
                if (byte_stride >= sizeof(uint8_t) * 2 && byte_stride * (vertices_count - 1) + sizeof(uint8_t) * 2 <= buffer_view.byteLength) {
                    for (size_t i = 0; i < vertices_count; i++) {
                        const auto* texcoord_data = reinterpret_cast<const uint8_t*>(buffer_data + i * byte_stride);
                        vertex_buffer[vertex_offset + i].u = texcoord_data[0] / 255.f;
                        vertex_buffer[vertex_offset + i].v = texcoord_data[1] / 255.f;
                    }
                } else {
                    throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
                }
            } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                size_t byte_stride = buffer_view.byteStride == 0 ? sizeof(uint16_t) * 2 : buffer_view.byteStride;
                if (byte_stride >= sizeof(uint16_t) * 2 && byte_stride * (vertices_count - 1) + sizeof(uint16_t) * 2 <= buffer_view.byteLength) {
                    for (size_t i = 0; i < vertices_count; i++) {
                        const auto* texcoord_data = reinterpret_cast<const uint16_t*>(buffer_data + i * byte_stride);
                        vertex_buffer[vertex_offset + i].u = texcoord_data[0] / 65535.f;
                        vertex_buffer[vertex_offset + i].v = texcoord_data[1] / 65535.f;
                    }
                } else {
                    throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
                }
            } else {
                throw std::runtime_error("Invalid TEXCOORD_0 accessor.");
            }
        }
    }

    if (attributes != 15) {
        std::string missing_attributes;
        if ((attributes & 1) == 0) {
            missing_attributes += "position";
        }
        if ((attributes & 2) == 0) {
            if (!missing_attributes.empty()) {
                missing_attributes += ", ";
            }
            missing_attributes += "normal";
        }
        if ((attributes & 4) == 0) {
            if (!missing_attributes.empty()) {
                missing_attributes += ", ";
            }
            missing_attributes += "tangent";
        }
        if ((attributes & 8) == 0) {
            if (!missing_attributes.empty()) {
                missing_attributes += ", ";
            }
            missing_attributes += "texcoord";
        }
        throw std::runtime_error(fmt::format("Missing attributes: {}", missing_attributes));
    }

    if (primitive.indices < 0 || primitive.indices >= model.accessors.size()) {
        throw std::runtime_error("Invalid index accessor.");
    }

    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
    if (accessor.type != TINYGLTF_TYPE_SCALAR ||
        accessor.sparse.isSparse ||
        accessor.count == 0 ||
        accessor.bufferView < 0 || accessor.bufferView >= model.bufferViews.size()) {
        throw std::runtime_error("Invalid index accessor.");
    }

    const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
    if (buffer_view.target != TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER ||
        buffer_view.buffer < 0 || buffer_view.buffer >= model.buffers.size()) {
        throw std::runtime_error("Invalid index accessor.");
    }

    const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];
    if (buffer_view.byteOffset + buffer_view.byteLength > buffer.data.size()) {
        throw std::runtime_error("Invalid index accessor.");
    }

    size_t index_offset = index_buffer.size();
    size_t indices_count = accessor.count;

    index_buffer.resize(index_offset + indices_count);

    const uint8_t* buffer_data = buffer.data.data() + buffer_view.byteOffset;
    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
        if ((buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint8_t)) &&
            buffer_view.byteLength == indices_count) {
            for (size_t i = 0; i < indices_count; i++) {
                index_buffer[index_offset + i] = static_cast<uint16_t>(vertex_offset + buffer_data[i]);
            }
        } else {
            throw std::runtime_error("Invalid BYTE index accessor.");
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        if ((buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint16_t)) &&
            buffer_view.byteLength == indices_count * sizeof(uint16_t)) {
            const auto* source_data = reinterpret_cast<const uint16_t*>(buffer_data);
            for (size_t i = 0; i < indices_count; i++) {
                index_buffer[index_offset + i] = static_cast<uint16_t>(vertex_offset + source_data[i]);
            }
        } else {
            throw std::runtime_error("Invalid SHORT index accessor.");
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        if ((buffer_view.byteStride == 0 || buffer_view.byteStride == sizeof(uint32_t)) &&
            buffer_view.byteLength == indices_count * sizeof(uint32_t)) {
            const auto* source_data = reinterpret_cast<const uint32_t*>(buffer_data);
            for (size_t i = 0; i < indices_count; i++) {
                index_buffer[index_offset + i] = static_cast<uint16_t>(vertex_offset + source_data[i]);
            }
        } else {
            throw std::runtime_error("Invalid INT index accessor.");
        }
    } else {
        throw std::runtime_error("Invalid index accessor.");
    }
}

void load_gltf_mesh(const tinygltf::Model& model, const tinygltf::Node& node, const glm::mat4& transform, std::vector<uint16_t>& index_buffer, std::vector<Vertex>& vertex_buffer) {
    const tinygltf::Mesh& mesh = model.meshes[node.mesh];
    for (const tinygltf::Primitive& primitive : mesh.primitives) {
        if (primitive.mode == TINYGLTF_MODE_TRIANGLES) {
            load_gltf_primitive(model, primitive, transform, index_buffer, vertex_buffer);
        }
    }
}

void load_gltf_node(const tinygltf::Model& model, const tinygltf::Node& node, const glm::mat4& parent_transform, std::vector<uint16_t>& index_buffer, std::vector<Vertex>& vertex_buffer) {
    glm::mat4 transform = parent_transform;
    glm::mat4 local_transform(1.f);

    if (node.matrix.size() == 16) {
        // Specified as 4x4 matrix.
        local_transform = glm::mat4(static_cast<float>(node.matrix[0]),  static_cast<float>(node.matrix[1]),
                                    static_cast<float>(node.matrix[2]),  static_cast<float>(node.matrix[3]),
                                    static_cast<float>(node.matrix[4]),  static_cast<float>(node.matrix[5]),
                                    static_cast<float>(node.matrix[6]),  static_cast<float>(node.matrix[7]),
                                    static_cast<float>(node.matrix[8]),  static_cast<float>(node.matrix[9]),
                                    static_cast<float>(node.matrix[10]), static_cast<float>(node.matrix[11]),
                                    static_cast<float>(node.matrix[12]), static_cast<float>(node.matrix[13]),
                                    static_cast<float>(node.matrix[14]), static_cast<float>(node.matrix[15]));
    } else {
        // Specified as translation, rotation and scale (all optional).

        if (node.rotation.size() == 4) {
            local_transform = glm::mat4_cast(glm::quat(static_cast<float>(node.rotation[3]),
                                                       static_cast<float>(-node.rotation[0]),
                                                       static_cast<float>(-node.rotation[1]),
                                                       static_cast<float>(node.rotation[2])));
        }

        if (node.scale.size() == 3) {
            local_transform = glm::scale(local_transform, glm::vec3(static_cast<float>(node.scale[0]),
                                                                    static_cast<float>(node.scale[1]),
                                                                    static_cast<float>(node.scale[2])));
        }

        if (node.translation.size() == 3) {
            local_transform = glm::translate(local_transform, glm::vec3(static_cast<float>(node.translation[0]),
                                                                        static_cast<float>(node.translation[1]),
                                                                        static_cast<float>(-node.translation[2])));
        }
    }

    transform *= local_transform;

    if (node.mesh >= 0 && node.mesh < model.meshes.size()) {
        load_gltf_mesh(model, node, transform, index_buffer, vertex_buffer);
    }

    for (int child_index : node.children) {
        if (child_index >= 0 && child_index < model.nodes.size()) {
            load_gltf_node(model, model.nodes[child_index], transform, index_buffer, vertex_buffer);
        } else {
            throw std::runtime_error("Invalid child index.");
        }
    }
}

} // namespace resource_geometry_details

bool ResourceGeometry::load(ResourceSingleComponent& resource_single_component, const std::string& path) {
    using namespace resource_geometry_details;

    std::string absolute_path = resource_single_component.get_absolute_path(path);

    try {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        std::string error, warning;
        if (loader.LoadBinaryFromFile(&model, &error, &warning, absolute_path)) {
            if (!warning.empty()) {
                bx::debugPrintf("glTF warning: %s\n", warning.c_str());
            }

            int scene_to_display = model.defaultScene > -1 ? model.defaultScene : 0;
            if (scene_to_display < model.scenes.size()) {
                std::vector<uint16_t> index_buffer;
                std::vector<Vertex> vertex_buffer;
                
                const tinygltf::Scene& scene = model.scenes[scene_to_display];
                for (int node_index : scene.nodes) {
                    if (node_index >= model.nodes.size()) {
                        throw std::runtime_error("Invalid node index.");
                    }

                    glm::mat4 transform(1.f);
                    load_gltf_node(model, model.nodes[node_index], transform, index_buffer, vertex_buffer);
                }

                m_indices_count = static_cast<uint32_t>(index_buffer.size());
                m_index_buffer = bgfx::createIndexBuffer(bgfx::copy(index_buffer.data(), m_indices_count * sizeof(uint16_t)));

                m_vertices_count = static_cast<uint32_t>(vertex_buffer.size());
                m_vertex_buffer = bgfx::createVertexBuffer(bgfx::copy(vertex_buffer.data(), m_vertices_count * sizeof(Vertex)), Vertex::DECLARATION);

                return true;
            } else {
                throw std::runtime_error("Invalid defaultScene value.");
            }
        } else {
            throw std::runtime_error(error);
        }
    } catch (const std::runtime_error& error) {
        bx::debugPrintf("Failed to load geometry file \"%s\".\nDetails: %s\n", absolute_path.c_str(), error.what());
    }

    return false;
}

} // namespace hg
