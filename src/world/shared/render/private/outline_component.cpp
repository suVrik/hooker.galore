#include "core/meta/registration.h"
#include "world/shared/render/outline_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<OutlineComponent>("OutlineComponent"_hs, std::make_pair("name"_hs, "OutlineComponent"), std::make_pair("ignore"_hs, true))
            .ctor<>();
}

} // namespace hg
