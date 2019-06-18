#include "core/meta/registration.h"
#include "world/shared/render/material_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<MaterialComponent>("MaterialComponent"_hs)
            .ctor<>()
            .data<&MaterialComponent::path>("path"_hs);
}

} // namespace hg
