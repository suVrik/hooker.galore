#pragma once

#include "core/ecs/system.h"

#include <entt/entity/observer.hpp>

namespace hg {

class PhysicsCharacterControllerComponent;
class PhysicsCharacterControllerSingleComponent;
class PhysicsSingleComponent;

/** `PhysicsCharacterControllerSystem` synchronizes World character controllers with PhysX character controllers. */
class PhysicsCharacterControllerSystem final : public FixedSystem {
public:
    explicit PhysicsCharacterControllerSystem(World& world);
    ~PhysicsCharacterControllerSystem() override;
    void update(float elapsed_time) override;

private:
    void character_controller_constructed(entt::entity entity, entt::registry& registry, PhysicsCharacterControllerComponent& physics_character_controller_component);
    void character_controller_destroyed(entt::entity entity, entt::registry& registry);
    void character_controller_private_destroyed(entt::entity entity, entt::registry& registry);

    PhysicsCharacterControllerSingleComponent& m_physics_character_controller_single_component;
    PhysicsSingleComponent& m_physics_single_component;

    entt::observer m_character_controller_observer;
    entt::observer m_transform_observer;
};

} // namespace hg
