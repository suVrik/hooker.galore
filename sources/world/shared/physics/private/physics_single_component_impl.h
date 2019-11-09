#pragma once

#include "world/shared/physics/physics_single_component.h"

namespace hg {

inline physx::PxPhysics& PhysicsSingleComponent::get_physics() const {
    assert(m_physics != nullptr);
    return *m_physics;
}

inline physx::PxCooking& PhysicsSingleComponent::get_cooking() const {
    assert(m_cooking != nullptr);
    return *m_cooking;
}

inline physx::PxScene& PhysicsSingleComponent::get_scene() const {
    assert(m_scene != nullptr);
    return *m_scene;
}

inline physx::PxMaterial* PhysicsSingleComponent::get_default_material() const {
    assert(m_default_material != nullptr);
    return m_default_material;
}

} // namespace hg
