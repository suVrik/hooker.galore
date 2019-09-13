#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/shared/physics/physics_box_shape_private_component.h"
#include "world/shared/physics/physics_rigid_body_system.h"
#include "world/shared/physics/physics_single_component.h"
#include "world/shared/physics/physics_static_rigid_body_component.h"
#include "world/shared/physics/physics_static_rigid_body_private_component.h"
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

namespace physics_rigid_body_system_details {

physx::PxTransform transform_component_to_physx_transform(const TransformComponent* const transform_component) noexcept {
    if (transform_component != nullptr) {
        const glm::vec3& translation = transform_component->translation;
        const glm::quat& rotation = transform_component->rotation;
        const physx::PxVec3 physx_translation(translation.x, translation.y, translation.z);
        const physx::PxQuat physx_rotation(rotation.x, rotation.y, rotation.z, rotation.w);
        return physx::PxTransform(physx_translation, physx_rotation);
    }
    return physx::PxTransform(physx::PxIdentity);
}

void attach_shapes_to_rigid_body(const entt::entity entity, entt::registry& registry, physx::PxRigidActor* const actor) noexcept {
    assert(actor != nullptr);

    const auto* const physics_box_shape_private_component = registry.try_get<PhysicsBoxShapePrivateComponent>(entity);
    if (physics_box_shape_private_component != nullptr) {
        assert(physics_box_shape_private_component->shape != nullptr);
        const bool result = actor->attachShape(*physics_box_shape_private_component->shape);
        assert(result);
    }
}

void detach_shapes_from_rigid_body(const entt::entity entity, entt::registry& registry, physx::PxRigidActor* const actor) noexcept {
    assert(actor != nullptr);

    const auto* const physics_box_shape_private_component = registry.try_get<PhysicsBoxShapePrivateComponent>(entity);
    if (physics_box_shape_private_component != nullptr) {
        assert(physics_box_shape_private_component->shape != nullptr);
        actor->detachShape(*physics_box_shape_private_component->shape);
    }
}

} // namespace physics_rigid_body_system_details

PhysicsRigidBodySystem::PhysicsRigidBodySystem(World& world)
        : FixedSystem(world)
        , m_physics_single_component(world.ctx<PhysicsSingleComponent>())
        , m_static_rigid_body_transform_observer(entt::observer(world, entt::collector.replace<TransformComponent>().where<PhysicsStaticRigidBodyPrivateComponent>().group<PhysicsStaticRigidBodyPrivateComponent, TransformComponent>())) {
    world.on_construct<PhysicsStaticRigidBodyComponent>().connect<&PhysicsRigidBodySystem::rigid_body_constructed>(*this);
    world.on_destroy<PhysicsStaticRigidBodyComponent>().connect<&PhysicsRigidBodySystem::rigid_body_destroyed>(*this);
    world.on_destroy<PhysicsStaticRigidBodyPrivateComponent>().connect<&PhysicsRigidBodySystem::rigid_body_private_destroyed>(*this);
}

PhysicsRigidBodySystem::~PhysicsRigidBodySystem() {
    // After disconnecting `on_destroy` callbacks, `PhysicsStaticRigidBodyPrivateComponent::rigid_actor` will never be released.
    world.view<PhysicsStaticRigidBodyPrivateComponent>().each([&](const entt::entity entity, PhysicsStaticRigidBodyPrivateComponent& /*physics_static_rigid_body_private_component*/) {
        world.remove<PhysicsStaticRigidBodyPrivateComponent>(entity);
    });

    world.on_construct<PhysicsStaticRigidBodyComponent>().disconnect<&PhysicsRigidBodySystem::rigid_body_constructed>(*this);
    world.on_destroy<PhysicsStaticRigidBodyComponent>().disconnect<&PhysicsRigidBodySystem::rigid_body_destroyed>(*this);
    world.on_destroy<PhysicsStaticRigidBodyPrivateComponent>().disconnect<&PhysicsRigidBodySystem::rigid_body_private_destroyed>(*this);
    m_static_rigid_body_transform_observer.disconnect();
}

void PhysicsRigidBodySystem::update(float /*elapsed_time*/) {
    using namespace physics_rigid_body_system_details;

    m_static_rigid_body_transform_observer.each([&](const entt::entity entity) {
        auto& physics_static_rigid_body_private_component = world.get<PhysicsStaticRigidBodyPrivateComponent>(entity);

        assert(physics_static_rigid_body_private_component.rigid_actor != nullptr);
        physics_static_rigid_body_private_component.rigid_actor->setGlobalPose(transform_component_to_physx_transform(world.try_get<TransformComponent>(entity)));
    });
}

void PhysicsRigidBodySystem::rigid_body_constructed(const entt::entity entity, entt::registry& registry, PhysicsStaticRigidBodyComponent& physics_static_rigid_body_component) noexcept {
    using namespace physics_rigid_body_system_details;

    assert(!registry.has<PhysicsStaticRigidBodyPrivateComponent>(entity));
    auto& physics_static_rigid_body_private_component = registry.assign<PhysicsStaticRigidBodyPrivateComponent>(entity);
    physics_static_rigid_body_private_component.rigid_actor = m_physics_single_component.physics->createRigidStatic(transform_component_to_physx_transform(world.try_get<TransformComponent>(entity)));
    assert(physics_static_rigid_body_private_component.rigid_actor != nullptr);

    physics_static_rigid_body_private_component.rigid_actor->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(entity));

    m_physics_single_component.scene->addActor(*physics_static_rigid_body_private_component.rigid_actor);

    attach_shapes_to_rigid_body(entity, registry, physics_static_rigid_body_private_component.rigid_actor);
}

void PhysicsRigidBodySystem::rigid_body_destroyed(const entt::entity entity, entt::registry& registry) noexcept {
    // `PhysicsStaticRigidBodyPrivateComponent` could be already removed when entity is being destroyed, because component destroy order is not specified.
    if (registry.has<PhysicsStaticRigidBodyPrivateComponent>(entity)) {
        registry.remove<PhysicsStaticRigidBodyPrivateComponent>(entity);
    }
}

void PhysicsRigidBodySystem::rigid_body_private_destroyed(const entt::entity entity, entt::registry& registry) noexcept {
    using namespace physics_rigid_body_system_details;
    
    auto& physics_static_rigid_body_private_component = registry.get<PhysicsStaticRigidBodyPrivateComponent>(entity);
    assert(physics_static_rigid_body_private_component.rigid_actor != nullptr);

    detach_shapes_from_rigid_body(entity, registry, physics_static_rigid_body_private_component.rigid_actor);

    physics_static_rigid_body_private_component.rigid_actor->release();
    physics_static_rigid_body_private_component.rigid_actor = nullptr;
}

} // namespace hg
