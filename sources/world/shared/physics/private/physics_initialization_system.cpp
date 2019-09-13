#include "core/ecs/system_descriptor.h"
#include "core/ecs/world.h"
#include "world/shared/physics/physics_initialization_system.h"
#include "world/shared/physics/physics_single_component.h"

#include <PxFoundation.h>
#include <PxPhysics.h>
#include <PxPhysicsVersion.h>
#include <PxSceneDesc.h>
#include <common/PxTolerancesScale.h>
#include <cooking/PxCooking.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include <extensions/PxExtensionsAPI.h>
#include <iostream>
#include <pvd/PxPvd.h>
#include <pvd/PxPvdTransport.h>

namespace hg {

SYSTEM_DESCRIPTOR(
    SYSTEM(PhysicsInitializationSystem),
    REQUIRE("physics")
)

PhysicsInitializationSystem::PhysicsInitializationSystem(World& world)
        : FixedSystem(world) {
    auto& physics_single_component = world.set<PhysicsSingleComponent>();

    physics_single_component.foundation = PxCreateFoundation(PX_PHYSICS_VERSION, physics_single_component.default_allocator_callback, physics_single_component.default_error_callback);
    if (physics_single_component.foundation == nullptr) {
        throw std::runtime_error("Failed to create PhysX foundation.");
    }

    try {
#ifndef NDEBUG
        physics_single_component.visual_debugger_transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        if (physics_single_component.visual_debugger_transport != nullptr) {
            physics_single_component.visual_debugger = physx::PxCreatePvd(*physics_single_component.foundation);
            if (!physics_single_component.visual_debugger->connect(*physics_single_component.visual_debugger_transport, physx::PxPvdInstrumentationFlag::eALL)) {
                std::cout << "[PHYSICS] Failed to connect to PhysX Visual Debugger." << std::endl;
            }
        } else {
            std::cout << "[PHYSICS] Failed to create PhysX Visual Debugger transport." << std::endl;
        }
#endif
        try {
            physx::PxTolerancesScale scale;
            physics_single_component.physics = PxCreatePhysics(PX_PHYSICS_VERSION, *physics_single_component.foundation, scale, false, physics_single_component.visual_debugger);
            if (physics_single_component.physics == nullptr) {
                throw std::runtime_error("Failed to create PhysX physics.");
            }

            try {
                if (!PxInitExtensions(*physics_single_component.physics, physics_single_component.visual_debugger)) {
                    throw std::runtime_error("Failed to init PhysX extensions.");
                }

                try {
                    physx::PxCookingParams cooking_params(scale);
                    physics_single_component.cooking = PxCreateCooking(PX_PHYSICS_VERSION, *physics_single_component.foundation, cooking_params);
                    if (physics_single_component.cooking == nullptr) {
                        throw std::runtime_error("Failed to create PhysX cooking.");
                    }

                    try {
                        physics_single_component.cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(0);
                        if (physics_single_component.cpu_dispatcher == nullptr) {
                            throw std::runtime_error("Failed to create PhysX CPU dispatcher.");
                        }

                        try {
                            physx::PxSceneDesc scene_description(scale);
                            scene_description.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);
                            scene_description.filterShader = physx::PxDefaultSimulationFilterShader; // TODO: Custom filter shader?
                            scene_description.cpuDispatcher = physics_single_component.cpu_dispatcher;
                            scene_description.simulationEventCallback = nullptr; // TODO: Custom event callback?
                            scene_description.flags = physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS | physx::PxSceneFlag::eENABLE_PCM | physx::PxSceneFlag::eENABLE_STABILIZATION;
                            
                            physics_single_component.scene = physics_single_component.physics->createScene(scene_description);
                            if (physics_single_component.scene == nullptr) {
                                throw std::runtime_error("Failed to create PhysX scene.");
                            }

                            physics_single_component.default_material = physics_single_component.physics->createMaterial(0.5f, 1.f, 0.1f);
                            assert(physics_single_component.default_material != nullptr);
                        } catch (...) {
                            physics_single_component.cpu_dispatcher->release();
                            throw;
                        }
                    } catch (...) {
                        physics_single_component.cooking->release();
                        throw;
                    }
                }
                catch (...) {
                    PxCloseExtensions();
                    throw;
                }
            } catch (...) {
                assert(physics_single_component.physics != nullptr);
                physics_single_component.physics->release();
                throw;
            }
        }
        catch (...) {
            if (physics_single_component.visual_debugger_transport != nullptr) {
                physics_single_component.visual_debugger_transport->release();
            }
            if (physics_single_component.visual_debugger != nullptr) {
                physics_single_component.visual_debugger->release();
            }
            throw;
        }
    }
    catch (...) {
        assert(physics_single_component.foundation != nullptr);
        physics_single_component.foundation->release();
        throw;
    }

    std::cout << "[PHYSICS] PhysX successfully initialized." << std::endl;
}

PhysicsInitializationSystem::~PhysicsInitializationSystem() {
    auto& physics_single_component = world.ctx<PhysicsSingleComponent>();

    assert(physics_single_component.scene != nullptr);
    physics_single_component.scene->release();

    assert(physics_single_component.cpu_dispatcher != nullptr);
    physics_single_component.cpu_dispatcher->release();

    assert(physics_single_component.cooking != nullptr);
    physics_single_component.cooking->release();
    
    PxCloseExtensions();

    assert(physics_single_component.physics != nullptr);
    physics_single_component.physics->release();

    if (physics_single_component.visual_debugger != nullptr) {
        physics_single_component.visual_debugger->release();
    }

    if (physics_single_component.visual_debugger_transport != nullptr) {
        physics_single_component.visual_debugger_transport->release();
    }

    assert(physics_single_component.foundation != nullptr);
    physics_single_component.foundation->release();
}

void PhysicsInitializationSystem::update(float /*elapsed_time*/) {
    // Nothing to do here.
}

} // namespace hg
