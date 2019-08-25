#include "core/meta/registration.h"
#include "world/shared/render/model_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<ModelComponent>("ModelComponent"_hs, std::make_pair("name"_hs, "ModelComponent"))
            .ctor<>()
            .data<&ModelComponent::path>("path"_hs, std::make_pair("name"_hs, "path"));
}

} // namespace hg
