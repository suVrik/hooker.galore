#include "core/meta/registration.h"

#include <glm/ext/quaternion_float.hpp>
#include <glm/vec3.hpp>
#include <string>

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<int32_t>("int32_t"_hs, std::make_pair("name"_hs, "int32_t"));
    entt::reflect<uint32_t>("uint32_t"_hs, std::make_pair("name"_hs, "uint32_t"));
    entt::reflect<float>("float"_hs, std::make_pair("name"_hs, "float"));
    entt::reflect<bool>("bool"_hs, std::make_pair("name"_hs, "bool"));
    entt::reflect<std::string>("std::string"_hs, std::make_pair("name"_hs, "std::string"));

    entt::reflect<glm::vec3>("vec3"_hs, std::make_pair("name"_hs, "vec3"))
            .ctor<>()
            .data<&glm::vec3::x>("x"_hs, std::make_pair("name"_hs, "x"))
            .data<&glm::vec3::y>("y"_hs, std::make_pair("name"_hs, "y"))
            .data<&glm::vec3::z>("z"_hs, std::make_pair("name"_hs, "z"));

    entt::reflect<glm::quat>("quat"_hs, std::make_pair("name"_hs, "quat"))
            .ctor<>()
            .data<&glm::quat::x>("x"_hs, std::make_pair("name"_hs, "x"))
            .data<&glm::quat::y>("y"_hs, std::make_pair("name"_hs, "y"))
            .data<&glm::quat::z>("z"_hs, std::make_pair("name"_hs, "z"))
            .data<&glm::quat::w>("w"_hs, std::make_pair("name"_hs, "w"));
}

} // namespace hg
