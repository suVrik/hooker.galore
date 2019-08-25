#include "core/meta/registration.h"
#include "world/shared/render/light_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<LightComponent>("LightComponent"_hs, std::make_pair("name"_hs, "LightComponent"))
            .ctor<>()
            .data<&LightComponent::color>("color"_hs, std::make_pair("name"_hs, "color"));
}

} // namespace hg
