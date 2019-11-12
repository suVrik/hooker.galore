#pragma once

#include <cassert>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultErrorCallback.h>

namespace physx {

class PxCooking;
class PxDefaultCpuDispatcher;
class PxFoundation;
class PxMaterial;
class PxPhysics;
class PxPvd;
class PxPvdTransport;
class PxScene;

} // namespace physx

namespace hg {

class PhysicsInitializationSystem;

/** `PhysicsSingleComponent` contains PhysX state. */
class PhysicsSingleComponent final {
public:
    /** Return physics. */
    physx::PxPhysics& get_physics() const;
    
    /** Return physx cooking. */
    physx::PxCooking& get_cooking() const;
    
    /** Return physx scene. */
    physx::PxScene& get_scene() const;
    
    /** Return physx default material. */
    physx::PxMaterial* get_default_material() const;

private:
    physx::PxDefaultErrorCallback m_default_error_callback;
    physx::PxDefaultAllocator m_default_allocator_callback;
    physx::PxFoundation* m_foundation = nullptr;
    physx::PxPvdTransport* m_visual_debugger_transport = nullptr;
    physx::PxPvd* m_visual_debugger = nullptr;
    physx::PxPhysics* m_physics = nullptr;
    physx::PxCooking* m_cooking = nullptr;
    physx::PxDefaultCpuDispatcher* m_cpu_dispatcher = nullptr;
    physx::PxScene* m_scene = nullptr;
    physx::PxMaterial* m_default_material = nullptr;

    friend class PhysicsInitializationSystem;
};

} // namespace hg

#include "world/physics/private/physics_single_component_impl.h"
