#include "core/meta/registration.h"

#include <glm/ext/quaternion_float.hpp>
#include <glm/vec3.hpp>

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<glm::vec3>("vec3"_hs)
            .ctor<>()
            .data<&glm::vec3::x>("x"_hs)
            .data<&glm::vec3::y>("y"_hs)
            .data<&glm::vec3::z>("z"_hs);

    entt::reflect<glm::quat>("quat"_hs)
            .ctor<>()
            .data<&glm::quat::x>("x"_hs)
            .data<&glm::quat::y>("y"_hs)
            .data<&glm::quat::z>("z"_hs)
            .data<&glm::quat::w>("w"_hs);
}

} // namespace hg
