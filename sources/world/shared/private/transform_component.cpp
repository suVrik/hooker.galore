#include "core/meta/registration.h"
#include "world/shared/transform_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<TransformComponent>("TransformComponent"_hs, std::make_pair("name"_hs, "TransformComponent"))
            .ctor<>()
            .data<&TransformComponent::translation>("translation"_hs, std::make_pair("name"_hs, "translation"))
            .data<&TransformComponent::rotation>("rotation"_hs, std::make_pair("name"_hs, "rotation"))
            .data<&TransformComponent::scale>("scale"_hs, std::make_pair("name"_hs, "scale"));
}

} // namespace hg
