#include "core/meta/registration.h"
#include "world/shared/transform_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<TransformComponent>("TransformComponent"_hs)
            .ctor<>()
            .data<&TransformComponent::translation>("translation"_hs)
            .data<&TransformComponent::rotation>("rotation"_hs)
            .data<&TransformComponent::scale>("scale"_hs);
}

} // namespace hg
