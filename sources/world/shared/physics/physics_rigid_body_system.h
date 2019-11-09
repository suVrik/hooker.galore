#pragma once

#include "core/ecs/system.h"

#include <entt/entity/observer.hpp>

namespace hg {

class PhysicsSingleComponent;
class PhysicsStaticRigidBodyComponent;

/** `PhysicsRigidBodySystem` synchronizes the World rigid body transforms with PhysX rigid body transforms. */
class PhysicsRigidBodySystem final : public FixedSystem {
public:
    explicit PhysicsRigidBodySystem(World& world);
    ~PhysicsRigidBodySystem();
    void update(float elapsed_time) override;

private:
    void rigid_body_constructed(entt::entity entity, entt::registry& registry, PhysicsStaticRigidBodyComponent& physics_static_rigid_body_component);
    void rigid_body_destroyed(entt::entity entity, entt::registry& registry);
    void rigid_body_private_destroyed(entt::entity entity, entt::registry& registry);

    PhysicsSingleComponent& m_physics_single_component;

    entt::observer m_static_rigid_body_transform_observer;
};

} // namespace hg
