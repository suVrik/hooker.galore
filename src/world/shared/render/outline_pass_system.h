#pragma once

#include "core/ecs/system.h"
#include "core/resource/model.h"
#include "world/shared/render/model_component.h"
#include "world/shared/render/outline_component.h"
#include "world/shared/transform_component.h"

#include <entt/entity/group.hpp>

namespace hg {

class OutlinePassSingleComponent;

/** `OutlinePassSystem` performs outline pass for all objects with `OutlineComponent` and presents it on the screen. */
class OutlinePassSystem final : public NormalSystem {
public:
    explicit OutlinePassSystem(World& world) noexcept;
    ~OutlinePassSystem() override;
    void update(float elapsed_time) override;

private:
    void reset(OutlinePassSingleComponent& outline_pass_single_component, uint16_t width, uint16_t height) const noexcept;
    void draw_node(const OutlinePassSingleComponent& outline_pass_single_component, const Model::Node& node, const glm::mat4& transform) const noexcept;

    entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<ModelComponent, TransformComponent>, OutlineComponent> m_group;
};

} // namespace hg
