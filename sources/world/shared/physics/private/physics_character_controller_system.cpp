#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/shared/physics/physics_character_controller_component.h"
#include "world/shared/physics/physics_character_controller_private_component.h"
#include "world/shared/physics/physics_character_controller_single_component.h"
#include "world/shared/physics/physics_character_controller_system.h"
#include "world/shared/physics/physics_single_component.h"
#include "world/shared/physics/physics_utils.h"
#include "world/shared/transform_component.h"

#include <characterkinematic/PxCapsuleController.h>
#include <characterkinematic/PxControllerManager.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(PhysicsCharacterControllerSystem),
    REQUIRE("physics"),
    BEFORE("PhysicsSimulateSystem"),
    AFTER("PhysicsInitializationSystem")
)

PhysicsCharacterControllerSystem::PhysicsCharacterControllerSystem(World& world)
        : FixedSystem(world)
        , m_physics_character_controller_single_component(world.set<PhysicsCharacterControllerSingleComponent>())
        , m_physics_single_component(world.ctx<PhysicsSingleComponent>())
        , m_transform_observer(entt::observer(world, entt::collector.group<PhysicsCharacterControllerPrivateComponent, TransformComponent>()
                                                                    .replace<TransformComponent>().where<PhysicsCharacterControllerPrivateComponent, TransformComponent>()))
        , m_character_controller_observer(entt::observer(world, entt::collector.group<PhysicsCharacterControllerComponent, PhysicsCharacterControllerPrivateComponent>()
                                                                               .replace<PhysicsCharacterControllerComponent>().where<PhysicsCharacterControllerComponent, PhysicsCharacterControllerPrivateComponent>())) {
    m_physics_character_controller_single_component.m_character_controller_manager = PxCreateControllerManager(m_physics_single_component.get_scene());
    if (m_physics_character_controller_single_component.m_character_controller_manager == nullptr) {
        throw std::runtime_error("Failed to create PhysX character controller manager.");
    }

    world.on_construct<PhysicsCharacterControllerComponent>().connect<&PhysicsCharacterControllerSystem::character_controller_constructed>(*this);
    world.on_destroy<PhysicsCharacterControllerComponent>().connect<&PhysicsCharacterControllerSystem::character_controller_destroyed>(*this);
    world.on_destroy<PhysicsCharacterControllerPrivateComponent>().connect<&PhysicsCharacterControllerSystem::character_controller_private_destroyed>(*this);
}

PhysicsCharacterControllerSystem::~PhysicsCharacterControllerSystem() {
    world.reset<PhysicsCharacterControllerPrivateComponent>();

    world.on_construct<PhysicsCharacterControllerComponent>().disconnect<&PhysicsCharacterControllerSystem::character_controller_constructed>(*this);
    world.on_destroy<PhysicsCharacterControllerComponent>().disconnect<&PhysicsCharacterControllerSystem::character_controller_destroyed>(*this);
    world.on_destroy<PhysicsCharacterControllerPrivateComponent>().disconnect<&PhysicsCharacterControllerSystem::character_controller_private_destroyed>(*this);

    assert(m_physics_character_controller_single_component.m_character_controller_manager != nullptr);
    m_physics_character_controller_single_component.m_character_controller_manager->release();

    m_character_controller_observer.disconnect();
    m_transform_observer.disconnect();

    world.unset<PhysicsCharacterControllerSingleComponent>();
}

void PhysicsCharacterControllerSystem::update(float elapsed_time) {
    m_transform_observer.each([&](const entt::entity entity) {
        auto& [physics_character_controller_private_component, transform_component] = world.get<PhysicsCharacterControllerPrivateComponent, TransformComponent>(entity);
        physics_character_controller_private_component.m_controller->setFootPosition(glm_vec3_to_physx_extended(transform_component.translation));
    });

    m_character_controller_observer.each([&](const entt::entity entity) {
        auto& [physics_character_controller_component, physics_character_controller_private_component] = world.get<PhysicsCharacterControllerComponent, PhysicsCharacterControllerPrivateComponent>(entity);

        if (physics_character_controller_component.m_descriptor_changed) {
            assert(physics_character_controller_component.m_step_offset > 0.f);
            assert(physics_character_controller_component.m_step_offset < 2.f * physics_character_controller_component.m_radius + physics_character_controller_component.m_height);
            physics_character_controller_private_component.m_controller->setStepOffset(physics_character_controller_component.m_step_offset);

            assert(static_cast<uint32_t>(physics_character_controller_component.m_non_walkable_mode) < 2);
            physics_character_controller_private_component.m_controller->setNonWalkableMode(static_cast<physx::PxControllerNonWalkableMode::Enum>(physics_character_controller_component.m_non_walkable_mode));

            assert(physics_character_controller_component.m_contact_offset > 0.f);
            physics_character_controller_private_component.m_controller->setContactOffset(physics_character_controller_component.m_contact_offset);

            physics_character_controller_private_component.m_controller->setUpDirection(glm_vec3_to_physx_vec3(physics_character_controller_component.m_up_direction));

            assert(physics_character_controller_component.m_slope_limit >= 0.f);
            physics_character_controller_private_component.m_controller->setSlopeLimit(physics_character_controller_component.m_slope_limit);

            assert(physics_character_controller_component.m_radius > 0.f);
            physics_character_controller_private_component.m_controller->setRadius(physics_character_controller_component.m_radius);

            assert(physics_character_controller_component.m_height > 0.f);
            physics_character_controller_private_component.m_controller->resize(physics_character_controller_component.m_height);

            assert(static_cast<uint32_t>(physics_character_controller_component.m_climbing_mode) < 2);
            physics_character_controller_private_component.m_controller->setClimbingMode(static_cast<physx::PxCapsuleClimbingMode::Enum>(physics_character_controller_component.m_climbing_mode));
            
            physics_character_controller_component.m_descriptor_changed = false;
        }

        if (physics_character_controller_component.m_offset) {
            const physx::PxControllerCollisionFlags collision_flags = physics_character_controller_private_component.m_controller->move(glm_vec3_to_physx_vec3(*physics_character_controller_component.m_offset), 1e-5f, elapsed_time, physx::PxControllerFilters());
            physics_character_controller_component.m_is_grounded = (collision_flags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
            physics_character_controller_component.m_offset = std::nullopt;
        }

        if (auto* const transform_component = world.try_get<TransformComponent>(entity); transform_component != nullptr) {
            transform_component->translation = physx_extended_to_glm_vec3(physics_character_controller_private_component.m_controller->getFootPosition());
            world.notify<TransformComponent>(entity);
        }
    });

    // Discard TransformComponent updates by CharacterControllerComponent move requests.
    m_transform_observer.clear();
}

void PhysicsCharacterControllerSystem::character_controller_constructed(const entt::entity entity, entt::registry& registry, PhysicsCharacterControllerComponent& physics_character_controller_component) noexcept {
    physx::PxCapsuleControllerDesc capsule_descriptor;

    assert(physics_character_controller_component.m_step_offset > 0.f);
    assert(physics_character_controller_component.m_step_offset < 2.f * physics_character_controller_component.m_radius + physics_character_controller_component.m_height);
    capsule_descriptor.stepOffset = physics_character_controller_component.m_step_offset;

    assert(static_cast<uint32_t>(physics_character_controller_component.m_non_walkable_mode) < 2);
    capsule_descriptor.nonWalkableMode = static_cast<physx::PxControllerNonWalkableMode::Enum>(physics_character_controller_component.m_non_walkable_mode);

    assert(physics_character_controller_component.m_contact_offset > 0.f);
    capsule_descriptor.contactOffset = physics_character_controller_component.m_contact_offset;
    
    capsule_descriptor.upDirection = physx::PxVec3(physics_character_controller_component.m_up_direction.x, physics_character_controller_component.m_up_direction.y, physics_character_controller_component.m_up_direction.z);

    assert(physics_character_controller_component.m_slope_limit >= 0.f);
    capsule_descriptor.slopeLimit = physics_character_controller_component.m_slope_limit;

    assert(physics_character_controller_component.m_radius > 0.f);
    capsule_descriptor.radius = physics_character_controller_component.m_radius;

    assert(physics_character_controller_component.m_height > 0.f);
    capsule_descriptor.height = physics_character_controller_component.m_height;

    assert(static_cast<uint32_t>(physics_character_controller_component.m_climbing_mode) < 2);
    capsule_descriptor.climbingMode = static_cast<physx::PxCapsuleClimbingMode::Enum>(physics_character_controller_component.m_climbing_mode);
    
    capsule_descriptor.material = m_physics_single_component.get_default_material();
    
    capsule_descriptor.scaleCoeff = 1.f;
    assert(capsule_descriptor.isValid());

    assert(!registry.has<PhysicsCharacterControllerPrivateComponent>(entity));
    auto& physics_character_controller_private_component = registry.assign<PhysicsCharacterControllerPrivateComponent>(entity);
    physics_character_controller_private_component.m_controller = static_cast<physx::PxCapsuleController*>(m_physics_character_controller_single_component.m_character_controller_manager->createController(capsule_descriptor));
    assert(physics_character_controller_private_component.m_controller != nullptr);

    physics_character_controller_private_component.m_controller->setUserData(reinterpret_cast<void*>(static_cast<uintptr_t>(entity)));
}

void PhysicsCharacterControllerSystem::character_controller_destroyed(const entt::entity entity, entt::registry& registry) noexcept {
    registry.reset<PhysicsCharacterControllerPrivateComponent>(entity);
}

void PhysicsCharacterControllerSystem::character_controller_private_destroyed(const entt::entity entity, entt::registry& registry) noexcept {
    auto& physics_character_controller_private_component = world.get<PhysicsCharacterControllerPrivateComponent>(entity);
    assert(physics_character_controller_private_component.m_controller != nullptr);
    physics_character_controller_private_component.m_controller->release();
    physics_character_controller_private_component.m_controller = nullptr;
}

} // namespace hg
