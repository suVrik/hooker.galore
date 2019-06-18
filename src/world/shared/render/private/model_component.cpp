#include "core/meta/registration.h"
#include "world/shared/render/model_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<ModelComponent>("ModelComponent"_hs)
            .ctor<>()
            .data<&ModelComponent::path>("path"_hs);
}

} // namespace hg
