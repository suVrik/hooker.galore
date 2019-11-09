#pragma once

#include "core/ecs/system.h"
#include "core/resource/model.h"
#include "world/shared/render/material_component.h"
#include "world/shared/render/model_component.h"
#include "world/shared/transform_component.h"

#include <entt/entity/group.hpp>

namespace hg {

struct GeometryPassSingleComponent;

/** `GeometryPassSystem` performs geometry pass for all objects.
    The result is stored in `GeometryPassSingleComponent`. */
class GeometryPassSystem final : public NormalSystem {
public:
    explicit GeometryPassSystem(World& world);
    ~GeometryPassSystem() override;
    void update(float elapsed_time) override;

private:
    struct DrawNodeContext;

    void reset(GeometryPassSingleComponent& geometry_pass_single_component, uint16_t width, uint16_t height) const;
    void draw_node(const DrawNodeContext& context, const Model::Node& node, const glm::mat4& transform) const;

    entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<>, ModelComponent, MaterialComponent, TransformComponent> m_group;
};

} // namespace hg
