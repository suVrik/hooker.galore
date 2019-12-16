#pragma once

#include "core/ecs/system.h"
#include "core/resource/model.h"
#include "world/render/material_component.h"
#include "world/render/model_component.h"
#include "world/shared/transform_component.h"

#include <entt/entity/group.hpp>

namespace hg {

struct ShadowPassSingleComponent;

/** `ShadowPassSystem` performs lighting pass after geometry pass. */
class ShadowPassSystem final : public NormalSystem {
public:
    explicit ShadowPassSystem(World& world);
    ~ShadowPassSystem() override;
    void update(float elapsed_time) override;
    void reset(ShadowPassSingleComponent& shadow_pass_single_component, uint16_t width, uint16_t height) const;
private:
    void draw_node(bgfx::ProgramHandle program, const Model::Node& node, const glm::mat4& transform) const;
    entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<>, ModelComponent, MaterialComponent, TransformComponent> m_group;
};

} // namespace hg
