#include "core/meta/registration.h"
#include "world/resource/resource_geometry_component.h"

namespace hg {

REFLECTION_REGISTRATION{
    entt::reflect<ResourceGeometryComponent>("ResourceGeometryComponent")
            .data<&ResourceGeometryComponent::set_geometry, &ResourceGeometryComponent::get_geometry>("m_geometry");

// editor
// deserialize
// serialize

}

} // namespace hg
