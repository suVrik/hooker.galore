#pragma once

#include "core/ecs/system.h"

#include <entt/entity/observer.hpp>

namespace physx {

class PxBoxGeometry;

} // namespace physx

namespace hg {

class PhysicsBoxShapeComponent;
class PhysicsSingleComponent;
struct TransformComponent;

/** `PhysicsShapeSystem` synchronizes all shape components with PhysX shapes. */
class PhysicsShapeSystem final : public FixedSystem {
public:
    explicit PhysicsShapeSystem(World& world);
    ~PhysicsShapeSystem() override;
    void update(float elapsed_time) override;

private:
    void box_shape_constructed(entt::entity entity, entt::registry& registry, PhysicsBoxShapeComponent& physics_box_shape_component) noexcept;
    void box_shape_destroyed(entt::entity entity, entt::registry& registry) noexcept;
    void box_shape_private_destroyed(entt::entity entity, entt::registry& registry) noexcept;
    physx::PxBoxGeometry box_shape_component_to_physx_box_geometry(PhysicsBoxShapeComponent& physics_box_shape_component, const TransformComponent* transform_component) noexcept;

    PhysicsSingleComponent& m_physics_single_component;

    entt::observer m_box_shape_transform_observer;
};

} // namespace hg
