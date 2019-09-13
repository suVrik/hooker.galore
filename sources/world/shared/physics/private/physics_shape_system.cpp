#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/shared/physics/physics_box_shape_component.h"
#include "world/shared/physics/physics_box_shape_private_component.h"
#include "world/shared/physics/physics_shape_system.h"
#include "world/shared/physics/physics_single_component.h"
#include "world/shared/physics/physics_static_rigid_body_private_component.h"
#include "world/shared/transform_component.h"

#include <PxPhysics.h>
#include <PxRigidStatic.h>
#include <PxShape.h>
#include <geometry/PxBoxGeometry.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(PhysicsShapeSystem),
    REQUIRE("physics"),
    BEFORE("PhysicsSimulateSystem"),
    AFTER("PhysicsInitializationSystem")
)

namespace physics_shape_system_details {

physx::PxBoxGeometry box_shape_component_to_physx_box_geometry(PhysicsBoxShapeComponent& physics_box_shape_component, const TransformComponent* const transform_component) noexcept {
    if (transform_component != nullptr) {
        return physx::PxBoxGeometry(physics_box_shape_component.size.x * transform_component->scale.x / 2.f,
                                    physics_box_shape_component.size.y * transform_component->scale.y / 2.f,
                                    physics_box_shape_component.size.z * transform_component->scale.z / 2.f);
    }
    return physx::PxBoxGeometry(physics_box_shape_component.size.x / 2.f, physics_box_shape_component.size.y / 2.f, physics_box_shape_component.size.z / 2.f);
}

void attach_shape_to_rigid_body(const entt::entity entity, entt::registry& registry, physx::PxShape* const shape) noexcept {
    assert(shape != nullptr);

    const auto* const physics_static_rigid_body_private_component = registry.try_get<PhysicsStaticRigidBodyPrivateComponent>(entity);
    if (physics_static_rigid_body_private_component != nullptr) {
        assert(physics_static_rigid_body_private_component->rigid_actor != nullptr);
        [[maybe_unused]] const bool result = physics_static_rigid_body_private_component->rigid_actor->attachShape(*shape);
        assert(result);
    }
}

void detach_shape_from_rigid_body(const entt::entity entity, entt::registry& registry, physx::PxShape* const shape) noexcept {
    assert(shape != nullptr);

    const auto* const physics_static_rigid_body_private_component = registry.try_get<PhysicsStaticRigidBodyPrivateComponent>(entity);
    if (physics_static_rigid_body_private_component != nullptr) {
        assert(physics_static_rigid_body_private_component->rigid_actor != nullptr);
        physics_static_rigid_body_private_component->rigid_actor->detachShape(*shape);
    }
}

} // namespace physics_shape_system_details

PhysicsShapeSystem::PhysicsShapeSystem(World& world)
        : FixedSystem(world)
        , m_physics_single_component(world.ctx<PhysicsSingleComponent>())
        , m_box_shape_transform_observer(entt::observer(world, entt::collector.replace<TransformComponent>().where<PhysicsBoxShapeComponent, PhysicsBoxShapePrivateComponent>().group<PhysicsBoxShapeComponent, PhysicsBoxShapePrivateComponent, TransformComponent>())) {
    world.on_construct<PhysicsBoxShapeComponent>().connect<&PhysicsShapeSystem::box_shape_constructed>(*this);
    world.on_destroy<PhysicsBoxShapeComponent>().connect<&PhysicsShapeSystem::box_shape_destroyed>(*this);
    world.on_destroy<PhysicsBoxShapePrivateComponent>().connect<&PhysicsShapeSystem::box_shape_private_destroyed>(*this);
}

PhysicsShapeSystem::~PhysicsShapeSystem() {
    // After disconnecting `on_destroy` callbacks, `PhysicsBoxShapePrivateComponent::shape` will never be released.
    world.view<PhysicsBoxShapePrivateComponent>().each([&](const entt::entity entity, PhysicsBoxShapePrivateComponent& /*physics_box_shape_private_component*/) {
        world.remove<PhysicsBoxShapePrivateComponent>(entity);
    });

    world.on_construct<PhysicsBoxShapeComponent>().disconnect<&PhysicsShapeSystem::box_shape_constructed>(*this);
    world.on_destroy<PhysicsBoxShapeComponent>().disconnect<&PhysicsShapeSystem::box_shape_destroyed>(*this);
    world.on_destroy<PhysicsBoxShapePrivateComponent>().disconnect<&PhysicsShapeSystem::box_shape_private_destroyed>(*this);
    m_box_shape_transform_observer.disconnect();
}

void PhysicsShapeSystem::update(float /*elapsed_time*/) {
    using namespace physics_shape_system_details;

    m_box_shape_transform_observer.each([&](const entt::entity entity) {
        auto& [physics_box_shape_component, physics_box_shape_private_component] = world.get<PhysicsBoxShapeComponent, PhysicsBoxShapePrivateComponent>(entity);

        assert(physics_box_shape_private_component.shape != nullptr);
        physics_box_shape_private_component.shape->setGeometry(box_shape_component_to_physx_box_geometry(physics_box_shape_component, world.try_get<TransformComponent>(entity)));
    });
}

void PhysicsShapeSystem::box_shape_constructed(const entt::entity entity, entt::registry& registry, PhysicsBoxShapeComponent& physics_box_shape_component) noexcept {
    using namespace physics_shape_system_details;

    assert(!registry.has<PhysicsBoxShapePrivateComponent>(entity));
    auto& physics_box_shape_private_component = registry.assign<PhysicsBoxShapePrivateComponent>(entity);
    physics_box_shape_private_component.shape = m_physics_single_component.physics->createShape(box_shape_component_to_physx_box_geometry(physics_box_shape_component, world.try_get<TransformComponent>(entity)), *m_physics_single_component.default_material, true);
    assert(physics_box_shape_private_component.shape != nullptr);

    physics_box_shape_private_component.shape->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(entity));

    attach_shape_to_rigid_body(entity, registry, physics_box_shape_private_component.shape);
}

void PhysicsShapeSystem::box_shape_destroyed(const entt::entity entity, entt::registry& registry) noexcept {
    // `PhysicsBoxShapePrivateComponent` could be already removed when entity is being destroyed, because component destroy order is not specified.
    if (registry.has<PhysicsBoxShapePrivateComponent>(entity)) {
        registry.remove<PhysicsBoxShapePrivateComponent>(entity);
    }
}

void PhysicsShapeSystem::box_shape_private_destroyed(const entt::entity entity, entt::registry& registry) noexcept {
    using namespace physics_shape_system_details;

    auto& physics_box_shape_private_component = world.get<PhysicsBoxShapePrivateComponent>(entity);
    assert(physics_box_shape_private_component.shape != nullptr);

    detach_shape_from_rigid_body(entity, registry, physics_box_shape_private_component.shape);

    physics_box_shape_private_component.shape->release();
    physics_box_shape_private_component.shape = nullptr;
}

} // namespace hg
