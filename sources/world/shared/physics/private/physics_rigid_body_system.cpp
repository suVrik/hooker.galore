#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/shared/physics/physics_box_shape_private_component.h"
#include "world/shared/physics/physics_rigid_body_system.h"
#include "world/shared/physics/physics_single_component.h"
#include "world/shared/physics/physics_static_rigid_body_component.h"
#include "world/shared/physics/physics_static_rigid_body_private_component.h"
#include "world/shared/physics/physics_utils.h"
#include "world/shared/transform_component.h"

#include <PxPhysics.h>
#include <PxRigidActor.h>
#include <PxRigidStatic.h>
#include <PxScene.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(PhysicsRigidBodySystem),
    REQUIRE("physics"),
    BEFORE("PhysicsSimulateSystem"),
    AFTER("PhysicsInitializationSystem")
)

PhysicsRigidBodySystem::PhysicsRigidBodySystem(World& world)
        : FixedSystem(world)
        , m_physics_single_component(world.ctx<PhysicsSingleComponent>())
        , m_static_rigid_body_transform_observer(entt::observer(world, entt::collector.group<PhysicsStaticRigidBodyPrivateComponent, TransformComponent>()
                                                                                      .replace<TransformComponent>().where<PhysicsStaticRigidBodyPrivateComponent>())) {
    world.on_construct<PhysicsStaticRigidBodyComponent>().connect<&PhysicsRigidBodySystem::rigid_body_constructed>(*this);
    world.on_destroy<PhysicsStaticRigidBodyComponent>().connect<&PhysicsRigidBodySystem::rigid_body_destroyed>(*this);
    world.on_destroy<PhysicsStaticRigidBodyPrivateComponent>().connect<&PhysicsRigidBodySystem::rigid_body_private_destroyed>(*this);
}

PhysicsRigidBodySystem::~PhysicsRigidBodySystem() {
    world.reset<PhysicsStaticRigidBodyPrivateComponent>();

    world.on_construct<PhysicsStaticRigidBodyComponent>().disconnect<&PhysicsRigidBodySystem::rigid_body_constructed>(*this);
    world.on_destroy<PhysicsStaticRigidBodyComponent>().disconnect<&PhysicsRigidBodySystem::rigid_body_destroyed>(*this);
    world.on_destroy<PhysicsStaticRigidBodyPrivateComponent>().disconnect<&PhysicsRigidBodySystem::rigid_body_private_destroyed>(*this);
    
    m_static_rigid_body_transform_observer.disconnect();
}

void PhysicsRigidBodySystem::update(float /*elapsed_time*/) {
    m_static_rigid_body_transform_observer.each([&](const entt::entity entity) {
        auto& physics_static_rigid_body_private_component = world.get<PhysicsStaticRigidBodyPrivateComponent>(entity);

        assert(physics_static_rigid_body_private_component.m_rigid_actor != nullptr);
        physics_static_rigid_body_private_component.m_rigid_actor->setGlobalPose(transform_component_to_physx_transform(world.try_get<TransformComponent>(entity)));
    });
}

void PhysicsRigidBodySystem::rigid_body_constructed(const entt::entity entity, entt::registry& registry, PhysicsStaticRigidBodyComponent& physics_static_rigid_body_component) {
    assert(!registry.has<PhysicsStaticRigidBodyPrivateComponent>(entity));
    auto& physics_static_rigid_body_private_component = registry.assign<PhysicsStaticRigidBodyPrivateComponent>(entity);
    physics_static_rigid_body_private_component.m_rigid_actor = m_physics_single_component.get_physics().createRigidStatic(transform_component_to_physx_transform(world.try_get<TransformComponent>(entity)));
    assert(physics_static_rigid_body_private_component.m_rigid_actor != nullptr);

    physics_static_rigid_body_private_component.m_rigid_actor->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(entity));

    m_physics_single_component.get_scene().addActor(*physics_static_rigid_body_private_component.m_rigid_actor);

    const auto* const physics_box_shape_private_component = registry.try_get<PhysicsBoxShapePrivateComponent>(entity);
    if (physics_box_shape_private_component != nullptr) {
        assert(physics_box_shape_private_component->m_shape != nullptr);
        [[maybe_unused]] const bool result = physics_static_rigid_body_private_component.m_rigid_actor->attachShape(*physics_box_shape_private_component->m_shape);
        assert(result);
    }
}

void PhysicsRigidBodySystem::rigid_body_destroyed(const entt::entity entity, entt::registry& registry) {
    registry.reset<PhysicsStaticRigidBodyPrivateComponent>(entity);
}

void PhysicsRigidBodySystem::rigid_body_private_destroyed(const entt::entity entity, entt::registry& registry) {
    auto& physics_static_rigid_body_private_component = registry.get<PhysicsStaticRigidBodyPrivateComponent>(entity);
    assert(physics_static_rigid_body_private_component.m_rigid_actor != nullptr);

    const auto* const physics_box_shape_private_component = registry.try_get<PhysicsBoxShapePrivateComponent>(entity);
    if (physics_box_shape_private_component != nullptr) {
        assert(physics_box_shape_private_component->m_shape != nullptr);
        physics_static_rigid_body_private_component.m_rigid_actor->detachShape(*physics_box_shape_private_component->m_shape);
    }

    physics_static_rigid_body_private_component.m_rigid_actor->release();
    physics_static_rigid_body_private_component.m_rigid_actor = nullptr;
}

} // namespace hg
