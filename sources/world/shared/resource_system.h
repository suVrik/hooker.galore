#pragma once

#include "core/ecs/system.h"

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

class Texture;

/** `ResourceSystem` loads all the resources asynchronously. */
class ResourceSystem final : public NormalSystem {
public:
    explicit ResourceSystem(World& world);
    ~ResourceSystem() override;
    void update(float elapsed_time) override;

private:
    void load_textures() const;
    Texture load_texture(const std::string &path) const;

    void load_presets() const;
    void load_preset(std::vector<entt::meta_any>& result, const std::string &path) const;

    entt::observer m_model_observer;
    entt::observer m_model_update_observer;
    entt::observer m_material_observer;
    entt::observer m_material_update_observer;
};

} // namespace hg
