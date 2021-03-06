#include "core/meta/registration.h"
#include "world/physics/physics_static_rigid_body_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<PhysicsStaticRigidBodyComponent>("PhysicsStaticRigidBodyComponent");
}

} // namespace hg
