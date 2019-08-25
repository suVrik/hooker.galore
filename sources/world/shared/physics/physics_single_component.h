#pragma once

#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultErrorCallback.h>

namespace physx {

class PxCooking;
class PxDefaultCpuDispatcher;
class PxFoundation;
class PxPhysics;
class PxPvd;
class PxPvdTransport;

} // namespace physx

namespace hg {

/** `PhysicsSingleComponent` contains PhysX state. */
struct PhysicsSingleComponent final {
    physx::PxDefaultErrorCallback default_error_callback;
    physx::PxDefaultAllocator default_allocator_callback;
    physx::PxFoundation* foundation = nullptr;
    physx::PxPvdTransport* visual_debugger_transport = nullptr;
    physx::PxPvd* visual_debugger = nullptr;
    physx::PxPhysics* physics = nullptr;
    physx::PxCooking* cooking = nullptr;
    physx::PxDefaultCpuDispatcher* cpu_dispatcher  = nullptr;
};

} // namespace hg
