#include "core/meta/registration.h"

#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <string>

namespace hg {

namespace registration_details {
    
bool edit_int32_t(const char* const name, entt::meta_handle value_handle) {
    assert(value_handle.type() == entt::resolve<int32_t>());

    auto* const value = value_handle.data<int32_t>();
    assert(value != nullptr);

    return ImGui::InputInt(name, value, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
}
    
bool edit_uint32_t(const char* const name, entt::meta_handle value_handle) {
    assert(value_handle.type() == entt::resolve<uint32_t>());

    uint32_t* const value = value_handle.data<uint32_t>();
    assert(value != nullptr);

    int32_t signed_value = *value;
    if (ImGui::InputInt(name, &signed_value, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
        *value = static_cast<uint32_t>(glm::max(signed_value, 0));
        return true;
    }
    return false;
}

bool edit_float(const char* const name, entt::meta_handle value_handle) {
    assert(value_handle.type() == entt::resolve<float>());

    auto* const value = value_handle.data<float>();
    assert(value != nullptr);

    return ImGui::InputFloat(name, value, 0.f, 0.f, "%.5f", ImGuiInputTextFlags_EnterReturnsTrue);
}
    
bool edit_bool(const char* const name, entt::meta_handle value_handle) {
    assert(value_handle.type() == entt::resolve<bool>());

    auto* const value = value_handle.data<bool>();
    assert(value != nullptr);

    return ImGui::Checkbox(name, value);
}

bool edit_string(const char* const name, entt::meta_handle value_handle) {
    assert(value_handle.type() == entt::resolve<std::string>());

    auto* const value = value_handle.data<std::string>();
    assert(value != nullptr);

    static char buffer[1024];
    std::strcpy(buffer, value->data());
    if (ImGui::InputText(name, buffer, std::size(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        *value = buffer;
        return true;
    }
    return false;
}
    
bool edit_vec2(const char* const name, entt::meta_handle value_handle) {
    assert(value_handle.type() == entt::resolve<glm::vec2>());

    auto* const value = value_handle.data<glm::vec2>();
    assert(value != nullptr);

    return ImGui::InputFloat2(name, glm::value_ptr(*value), "%.5f", ImGuiInputTextFlags_EnterReturnsTrue);
}
    
bool edit_vec3(const char* const name, entt::meta_handle value_handle) {
    assert(value_handle.type() == entt::resolve<glm::vec3>());

    auto* const value = value_handle.data<glm::vec3>();
    assert(value != nullptr);

    return ImGui::InputFloat3(name, glm::value_ptr(*value), "%.5f", ImGuiInputTextFlags_EnterReturnsTrue);
}
    
bool edit_vec4(const char* const name, entt::meta_handle value_handle) {
    assert(value_handle.type() == entt::resolve<glm::vec4>());

    auto* const value = value_handle.data<glm::vec4>();
    assert(value != nullptr);

    return ImGui::InputFloat4(name, glm::value_ptr(*value), "%.5f", ImGuiInputTextFlags_EnterReturnsTrue);
}
    
bool edit_quat(const char* const name, entt::meta_handle value_handle) {
    assert(value_handle.type() == entt::resolve<glm::quat>());

    auto* const value = value_handle.data<glm::quat>();
    assert(value != nullptr);

    return ImGui::InputFloat4(name, glm::value_ptr(*value), "%.5f", ImGuiInputTextFlags_EnterReturnsTrue);
}

template <typename T>
bool basic_compare(const entt::meta_handle lhs_handle, const entt::meta_handle rhs_handle) {
    assert(lhs_handle.type() == entt::resolve<T>());
    assert(rhs_handle.type() == entt::resolve<T>());

    const auto* const lhs_value = lhs_handle.data<T>();
    assert(lhs_value != nullptr);

    const auto* const rhs_value = rhs_handle.data<T>();
    assert(rhs_value != nullptr);

    return *lhs_value == *rhs_value;
}

template <typename T>
bool epsilon_compare(const entt::meta_handle lhs_handle, const entt::meta_handle rhs_handle) {
    assert(lhs_handle.type() == entt::resolve<T>());
    assert(rhs_handle.type() == entt::resolve<T>());

    const auto* const lhs_value = lhs_handle.data<T>();
    assert(lhs_value != nullptr);

    const auto* const rhs_value = rhs_handle.data<T>();
    assert(rhs_value != nullptr);

    if constexpr (std::is_same_v<T, float>) {
        return std::abs(*lhs_value - *rhs_value) < glm::epsilon<float>();
    } else {
        return glm::all(glm::equal(*lhs_value, *rhs_value, glm::epsilon<float>()));
    }
}

} // namespace registration_details

REFLECTION_REGISTRATION {
    entt::reflect<int32_t>("int32_t")
            .func<&registration_details::edit_int32_t>("editor")
            .func<&registration_details::basic_compare<int32_t>>("compare");

    entt::reflect<uint32_t>("uint32_t")
            .func<&registration_details::edit_uint32_t>("editor")
            .func<&registration_details::basic_compare<uint32_t>>("compare");

    entt::reflect<float>("float")
            .func<&registration_details::edit_float>("editor")
            .func<&registration_details::epsilon_compare<float>>("compare");

    entt::reflect<bool>("bool")
            .func<&registration_details::edit_bool>("editor")
            .func<&registration_details::basic_compare<bool>>("compare");

    entt::reflect<std::string>("string")
            .func<&registration_details::edit_string>("editor")
            .func<&registration_details::basic_compare<std::string>>("compare");

    entt::reflect<glm::vec2>("vec2")
            .ctor<>()
            .data<&glm::vec2::x>("x")
            .data<&glm::vec2::y>("y")
            .func<&registration_details::edit_vec2>("editor")
            .func<&registration_details::epsilon_compare<glm::vec2>>("compare");

    entt::reflect<glm::vec3>("vec3")
            .ctor<>()
            .data<&glm::vec3::x>("x")
            .data<&glm::vec3::y>("y")
            .data<&glm::vec3::z>("z")
            .func<&registration_details::edit_vec3>("editor")
            .func<&registration_details::epsilon_compare<glm::vec3>>("compare");

    entt::reflect<glm::vec4>("vec4")
            .ctor<>()
            .data<&glm::vec4::x>("x")
            .data<&glm::vec4::y>("y")
            .data<&glm::vec4::z>("z")
            .data<&glm::vec4::w>("w")
            .func<&registration_details::edit_vec4>("editor")
            .func<&registration_details::epsilon_compare<glm::vec4>>("compare");

    entt::reflect<glm::quat>("quat")
            .ctor<>()
            .data<&glm::quat::x>("x")
            .data<&glm::quat::y>("y")
            .data<&glm::quat::z>("z")
            .data<&glm::quat::w>("w")
            .func<&registration_details::edit_quat>("editor")
            .func<&registration_details::epsilon_compare<glm::quat>>("compare");
}

} // namespace hg
