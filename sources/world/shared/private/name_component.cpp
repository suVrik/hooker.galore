#include "core/meta/registration.h"
#include "world/shared/name_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<NameComponent>("NameComponent")
            .data<&NameComponent::name>("name");
}

} // namespace hg
