#include "core/meta/registration.h"
#include "world/render/light_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<LightComponent>("LightComponent")
            .data<&LightComponent::color>("color");
}

} // namespace hg
