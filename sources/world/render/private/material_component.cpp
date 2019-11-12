#include "core/meta/registration.h"
#include "world/render/material_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<MaterialComponent>("MaterialComponent")
            .data<&MaterialComponent::material>("material");
}

} // namespace hg
