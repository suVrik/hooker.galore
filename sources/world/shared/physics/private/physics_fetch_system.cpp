#include "core/ecs/world.h"
#include "world/shared/physics/physics_fetch_system.h"
#include "world/shared/physics/physics_single_component.h"

#include <PxFoundation.h>
#include <PxPhysics.h>
#include <PxPhysicsVersion.h>
#include <common/PxTolerancesScale.h>
#include <extensions/PxExtensionsAPI.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <iostream>
#include <pvd/PxPvd.h>
#include <pvd/PxPvdTransport.h>
#include <cooking/PxCooking.h>

namespace hg {

// TODO: Actually `PhysicsFetchSystem` is a `FixedSystem`. Keep it `NormalSystem` until physics is stable.
PhysicsFetchSystem::PhysicsFetchSystem(World& world)
        : NormalSystem(world) {
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
            if (!physics_single_component.visual_debugger->connect(*physics_single_component.visual_debugger_transport, physx::PxPvdInstrumentationFlag::ePROFILE)) {
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

PhysicsFetchSystem::~PhysicsFetchSystem() {
    auto& physics_single_component = world.ctx<PhysicsSingleComponent>();

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

void PhysicsFetchSystem::update(float /*elapsed_time*/) {
    //
}

} // namespace hg
