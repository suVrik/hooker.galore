#include "core/meta/registration.h"
#include "world/shared/render/material_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<MaterialComponent>("MaterialComponent"_hs, std::make_pair("name"_hs, "MaterialComponent"))
            .ctor<>()
            .data<&MaterialComponent::material>("material"_hs, std::make_pair("name"_hs, "material"))
            .data<&MaterialComponent::parallax_scale>("parallax_scale"_hs, std::make_pair("name"_hs, "parallax_scale"))
            .data<&MaterialComponent::parallax_steps>("parallax_steps"_hs, std::make_pair("name"_hs, "parallax_steps"));
}

} // namespace hg
