#include "core/meta/registration.h"
#include "world/shared/render/blockout_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<BlockoutComponent>("BlockoutComponent");
}

} // namespace hg
