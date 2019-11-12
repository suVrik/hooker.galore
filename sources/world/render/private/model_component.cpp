#include "core/meta/registration.h"
#include "world/render/model_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<ModelComponent>("ModelComponent")
            .data<&ModelComponent::path>("path");
}

} // namespace hg
