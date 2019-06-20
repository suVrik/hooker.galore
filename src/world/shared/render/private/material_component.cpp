#include "core/meta/registration.h"
#include "world/shared/render/material_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<MaterialComponent>("MaterialComponent"_hs, std::make_pair("name"_hs, "MaterialComponent"))
            .ctor<>()
            .data<&MaterialComponent::path>("path"_hs, std::make_pair("name"_hs, "path"));
}

} // namespace hg
