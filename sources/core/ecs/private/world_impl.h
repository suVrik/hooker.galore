#pragma once

#include "core/ecs/world.h"

namespace hg {

template <typename T>
void World::each_registered_component(const entt::entity entity, T callback) const noexcept {
    ComponentManager::each_registered([&](const entt::meta_type component_type) {
        const entt::meta_handle component_handle = get(entity, component_type);
        if (component_handle) {
            callback(component_handle);
        }
    });
}

template <typename T>
void World::each_editable_component(const entt::entity entity, T callback) const noexcept {
    ComponentManager::each_editable([&](const entt::meta_type component_type) {
        const entt::meta_handle component_handle = get(entity, component_type);
        if (component_handle) {
            callback(component_handle);
        }
    });
}

template <typename... Tags>
void World::add_tags(Tags&& ... tags) noexcept {
    (add_tag(tags), ...);
}

template <typename... Tags>
void World::remove_tags(Tags&& ... tags) noexcept {
    (remove_tag(tags), ...);
}

template <typename... Tags>
bool World::check_tags(Tags&& ... tags) noexcept {
    return (check_tag(tags) && ...);
}

} // namespace hg
