#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/shared/physics/physics_box_shape_component.h"
#include "world/shared/physics/physics_box_shape_private_component.h"
#include "world/shared/physics/physics_shape_system.h"
#include "world/shared/physics/physics_single_component.h"
#include "world/shared/physics/physics_static_rigid_body_private_component.h"
#include "world/shared/physics/physics_tags.h"
#include "world/shared/transform_component.h"

#include <PxPhysics.h>
#include <PxRigidStatic.h>
#include <PxShape.h>
#include <geometry/PxBoxGeometry.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(PhysicsShapeSystem),
    TAGS(physics),
    BEFORE("PhysicsSimulateSystem"),
    AFTER("PhysicsInitializationSystem")
)

PhysicsShapeSystem::PhysicsShapeSystem(World& world)
        : FixedSystem(world)
        , m_physics_single_component(world.ctx<PhysicsSingleComponent>())
        , m_box_shape_transform_observer(entt::observer(world, entt::collector.group<PhysicsBoxShapeComponent, PhysicsBoxShapePrivateComponent, TransformComponent>()
                                                                              .replace<TransformComponent>().where<PhysicsBoxShapeComponent, PhysicsBoxShapePrivateComponent, TransformComponent>()
                                                                              .replace<PhysicsBoxShapeComponent>().where<PhysicsBoxShapeComponent, PhysicsBoxShapePrivateComponent, TransformComponent>())) {
    world.on_construct<PhysicsBoxShapeComponent>().connect<&PhysicsShapeSystem::box_shape_constructed>(*this);
    world.on_destroy<PhysicsBoxShapeComponent>().connect<&PhysicsShapeSystem::box_shape_destroyed>(*this);
    world.on_destroy<PhysicsBoxShapePrivateComponent>().connect<&PhysicsShapeSystem::box_shape_private_destroyed>(*this);
}

PhysicsShapeSystem::~PhysicsShapeSystem() {
    world.reset<PhysicsBoxShapePrivateComponent>();

    world.on_construct<PhysicsBoxShapeComponent>().disconnect<&PhysicsShapeSystem::box_shape_constructed>(*this);
    world.on_destroy<PhysicsBoxShapeComponent>().disconnect<&PhysicsShapeSystem::box_shape_destroyed>(*this);
    world.on_destroy<PhysicsBoxShapePrivateComponent>().disconnect<&PhysicsShapeSystem::box_shape_private_destroyed>(*this);
    
    m_box_shape_transform_observer.disconnect();
}

void PhysicsShapeSystem::update(float /*elapsed_time*/) {
    m_box_shape_transform_observer.each([&](const entt::entity entity) {
        auto& [physics_box_shape_component, physics_box_shape_private_component, transform_component] = world.get<PhysicsBoxShapeComponent, PhysicsBoxShapePrivateComponent, TransformComponent>(entity);

        assert(physics_box_shape_private_component.m_shape != nullptr);
        physics_box_shape_private_component.m_shape->setGeometry(box_shape_component_to_physx_box_geometry(physics_box_shape_component, &transform_component));
    });
}

void PhysicsShapeSystem::box_shape_constructed(const entt::entity entity, entt::registry& registry, PhysicsBoxShapeComponent& physics_box_shape_component) {
    assert(!registry.has<PhysicsBoxShapePrivateComponent>(entity));
    auto& physics_box_shape_private_component = registry.assign<PhysicsBoxShapePrivateComponent>(entity);
    physics_box_shape_private_component.m_shape = m_physics_single_component.get_physics().createShape(box_shape_component_to_physx_box_geometry(physics_box_shape_component, world.try_get<TransformComponent>(entity)), *m_physics_single_component.get_default_material(), true);
    assert(physics_box_shape_private_component.m_shape != nullptr);

    physics_box_shape_private_component.m_shape->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(entity));

    const auto* const physics_static_rigid_body_private_component = registry.try_get<PhysicsStaticRigidBodyPrivateComponent>(entity);
    if (physics_static_rigid_body_private_component != nullptr) {
        assert(physics_static_rigid_body_private_component->m_rigid_actor != nullptr);
        [[maybe_unused]] const bool result = physics_static_rigid_body_private_component->m_rigid_actor->attachShape(*physics_box_shape_private_component.m_shape);
        assert(result);
    }
}

void PhysicsShapeSystem::box_shape_destroyed(const entt::entity entity, entt::registry& registry) {
    registry.reset<PhysicsBoxShapePrivateComponent>(entity);
}

void PhysicsShapeSystem::box_shape_private_destroyed(const entt::entity entity, entt::registry& registry) {
    auto& physics_box_shape_private_component = world.get<PhysicsBoxShapePrivateComponent>(entity);
    assert(physics_box_shape_private_component.m_shape != nullptr);

    const auto* const physics_static_rigid_body_private_component = registry.try_get<PhysicsStaticRigidBodyPrivateComponent>(entity);
    if (physics_static_rigid_body_private_component != nullptr) {
        assert(physics_static_rigid_body_private_component->m_rigid_actor != nullptr);
        physics_static_rigid_body_private_component->m_rigid_actor->detachShape(*physics_box_shape_private_component.m_shape);
    }

    physics_box_shape_private_component.m_shape->release();
    physics_box_shape_private_component.m_shape = nullptr;
}

physx::PxBoxGeometry PhysicsShapeSystem::box_shape_component_to_physx_box_geometry(PhysicsBoxShapeComponent& physics_box_shape_component, const TransformComponent* const transform_component) {
    glm::vec3 box_size = physics_box_shape_component.get_size() / 2.f;

    if (transform_component != nullptr) {
        box_size = glm::max(box_size * transform_component->scale, glm::vec3(glm::epsilon<float>(), glm::epsilon<float>(), glm::epsilon<float>()));
    }

    assert(box_size.x > 0.f);
    assert(box_size.y > 0.f);
    assert(box_size.z > 0.f);

    return physx::PxBoxGeometry(box_size.x, box_size.y, box_size.z);
}

} // namespace hg
