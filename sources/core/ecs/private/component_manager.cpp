#include "core/ecs/component_manager.h"

#include <entt/meta/factory.hpp>

namespace hg {

std::unordered_map<entt::meta_type, ComponentManager::ComponentDescriptor> ComponentManager::descriptors;

entt::meta_any ComponentManager::construct(entt::meta_type component_type) {
    assert(is_registered(component_type));
    assert(is_default_constructible(component_type));
    return descriptors.find(component_type)->second.construct();
}

entt::meta_any ComponentManager::copy(entt::meta_handle component) {
    assert(is_registered(component.type()));
    assert(is_copy_constructible(component.type()));
    return descriptors.find(component.type())->second.copy(component);
}

entt::meta_any ComponentManager::move(entt::meta_handle component) {
    assert(is_registered(component.type()));
    assert(is_move_constructible(component.type()));
    return descriptors.find(component.type())->second.move(component);
}

entt::meta_any ComponentManager::move_or_copy(entt::meta_handle component) {
    if (is_move_constructible(component.type())) {
        return move(component);
    }
    return copy(component);
}

const char* ComponentManager::get_name(entt::meta_type component_type, const char* fallback) {
    assert(is_registered(component_type));

    entt::meta_prop component_name_property = component_type.prop("name"_hs);
    if (component_name_property && component_name_property.value().type() == entt::resolve<const char*>()) {
        return component_name_property.value().cast<const char*>();
    }
    return fallback;
}

bool ComponentManager::is_registered(entt::meta_type component_type) {
    return descriptors.count(component_type) != 0;
}

bool ComponentManager::is_default_constructible(entt::meta_type component_type) {
    assert(is_registered(component_type));
    assert((descriptors.find(component_type)->second.assign_default == nullptr) == (descriptors.find(component_type)->second.get_or_assign == nullptr));
    return descriptors.find(component_type)->second.assign_default != nullptr;
}

bool ComponentManager::is_copy_constructible(entt::meta_type component_type) {
    assert(is_registered(component_type));
    assert((descriptors.find(component_type)->second.assign_copy != nullptr) == (descriptors.find(component_type)->second.copy != nullptr));
    return descriptors.find(component_type)->second.assign_copy != nullptr;
}

bool ComponentManager::is_move_constructible(entt::meta_type component_type) {
    assert(is_registered(component_type));
    assert((descriptors.find(component_type)->second.assign_move != nullptr) == (descriptors.find(component_type)->second.move != nullptr));
    return descriptors.find(component_type)->second.assign_move != nullptr;
}

bool ComponentManager::is_copy_assignable(entt::meta_type component_type) {
    assert(is_registered(component_type));
    return descriptors.find(component_type)->second.replace_copy != nullptr;
}

bool ComponentManager::is_move_assignable(entt::meta_type component_type) {
    assert(is_registered(component_type));
    return descriptors.find(component_type)->second.replace_move != nullptr;
}

bool ComponentManager::is_ignored(entt::meta_type component_type) {
    assert(is_registered(component_type));

    entt::meta_prop ignore_property = component_type.prop("ignore"_hs);
    return ignore_property && ignore_property.value().type() == entt::resolve<bool>() && ignore_property.value().cast<bool>();
}

bool ComponentManager::is_editable(const entt::meta_type component_type) {
    return is_registered(component_type) &&
           get_name(component_type, nullptr) != nullptr &&
           !is_ignored(component_type) &&
           is_default_constructible(component_type) &&
           is_copy_constructible(component_type) &&
           is_copy_assignable(component_type);
}

} // namespace hg
