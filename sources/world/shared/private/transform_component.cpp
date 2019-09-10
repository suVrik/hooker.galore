#include "core/meta/registration.h"
#include "world/shared/transform_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<TransformComponent>("TransformComponent")
            .data<&TransformComponent::translation>("translation")
            .data<&TransformComponent::rotation>("rotation")
            .data<&TransformComponent::scale>("scale");
}

} // namespace hg
