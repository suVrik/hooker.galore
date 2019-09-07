#pragma once

#include "core/ecs/system.h"
#include "core/resource/model.h"

#include <entt/entity/observer.hpp>
#include <string>
#include <unordered_set>

namespace tinygltf {

class Model;
class Node;
struct Primitive;

} // namespace tinygltf

namespace entt {

class meta_any;

} // namespace entt

namespace hg {

/** `ResourceSystem` loads all the resources asynchronously. */
class ResourceSystem final : public NormalSystem {
public:
    explicit ResourceSystem(World& world);
    ~ResourceSystem() override;
    void update(float elapsed_time) override;

private:
    void load_textures() const;
    Texture load_texture(const std::string &path) const noexcept;

    void load_models() const;
    void load_model(Model& result, const std::string &path) const;
    void load_model_node(const glm::mat4& parent_transform, Model::Node& result, Model::AABB& bounds, const tinygltf::Model &model, const tinygltf::Node &node) const;
    void load_model_mesh(const glm::mat4& parent_transform, Model::Mesh& result, Model::AABB& bounds, const tinygltf::Model &model, const tinygltf::Node &node) const;
    void load_model_primitive(const glm::mat4& parent_transform, Model::Primitive& result, Model::AABB& bounds, const tinygltf::Model &model, const tinygltf::Primitive& primitive) const;

    void load_presets() const;
    void load_preset(std::vector<entt::meta_any>& result, const std::string &path) const;

    entt::observer m_model_observer;
    entt::observer m_model_update_observer;
    entt::observer m_material_observer;
    entt::observer m_material_update_observer;
};

} // namespace hg
