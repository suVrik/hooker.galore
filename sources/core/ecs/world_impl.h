#pragma once

#include "core/ecs/world.h"

namespace hg {

void register_components(World& world) noexcept;
void register_systems(World& world) noexcept;

template <typename T>
void World::register_component() noexcept {
    ComponentDescriptor descriptor{};

    if (std::is_default_constructible_v<T>) {
        descriptor.construct = []() -> entt::meta_any {
            return entt::meta_any(T{});
        };
    } else {
        descriptor.construct = nullptr;
    }
    
    if constexpr (std::is_copy_constructible_v<T>) {
        descriptor.copy = [](const entt::meta_handle component) -> entt::meta_any {
            return entt::meta_any(T(*component.data<T>()));
        };
    } else {
        descriptor.copy = nullptr;
    }

    if constexpr (std::is_move_constructible_v<T>) {
        descriptor.move = [](const entt::meta_handle component) -> entt::meta_any {
            return entt::meta_any(T(std::move(*component.data<T>())));
        };
    } else {
        descriptor.move = nullptr;
    }

    if constexpr (std::is_default_constructible_v<T>) {
        static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>, "EnTT requires component type to be either copy or move assignable as well.");
        descriptor.assign_default = [](World* const world, const entt::entity entity) -> entt::meta_handle {
            return entt::meta_handle(world->assign<T>(entity));
        };
    } else {
        descriptor.assign_default = nullptr;
    }

    if constexpr (std::is_copy_constructible_v<T>) {
        static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>, "EnTT requires component type to be either copy or move assignable as well.");
        descriptor.assign_copy = [](World* const world, const entt::entity entity, const entt::meta_handle component) -> entt::meta_handle {
            return entt::meta_handle(world->assign<T>(entity, *component.data<T>()));
        };
    } else {
        descriptor.assign_copy = nullptr;
    }

    if constexpr (std::is_move_constructible_v<T>) {
        static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>, "EnTT requires component type to be either copy or move assignable as well.");
        descriptor.assign_move = [](World* const world, const entt::entity entity, const entt::meta_handle component) -> entt::meta_handle {
            return entt::meta_handle(world->assign<T>(entity, std::move(*component.data<T>())));
        };
    } else {
        descriptor.assign_move = nullptr;
    }

    if constexpr (std::is_copy_assignable_v<T>) {
        descriptor.replace_copy = [](World* const world, const entt::entity entity, const entt::meta_handle component) -> entt::meta_handle {
            return entt::meta_handle(world->replace<T>(entity, *component.data<T>()));
        };
    } else {
        descriptor.replace_copy = nullptr;
    }

    if constexpr (std::is_move_assignable_v<T>) {
        descriptor.replace_move = [](World* const world, const entt::entity entity, const entt::meta_handle component) -> entt::meta_handle {
            return entt::meta_handle(world->replace<T>(entity, std::move(*component.data<T>())));
        };
    } else {
        descriptor.replace_move = nullptr;
    }

    descriptor.remove = [](World* const world, const entt::entity entity) {
        world->remove<T>(entity);
    };

    descriptor.has = [](const World* const world, const entt::entity entity) -> bool {
        return world->has<T>(entity);
    };

    descriptor.get = [](const World* const world, const entt::entity entity) -> entt::meta_handle {
        return entt::meta_handle(*const_cast<World*>(world)->try_get<T>(entity));
    };

    if (std::is_default_constructible_v<T>) {
        static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>, "EnTT requires component type to be either copy or move assignable as well.");
        descriptor.get_or_assign = [](World* const world, const entt::entity entity) -> entt::meta_handle {
            return entt::meta_handle(world->get_or_assign<T>(entity));
        };
    } else {
        descriptor.get_or_assign = nullptr;
    }

    m_components.emplace(entt::resolve<T>(), descriptor);
}

template <typename T>
void World::each_registered_component_type(T callback) const noexcept {
    for (const auto& [type, descriptor] : m_components) {
        callback(type);
    }
}

template <typename T>
void World::each_editable_component_type(T callback) const noexcept {
    for (const auto& [type, descriptor] : m_components) {
        if (is_component_editable(type)) {
            callback(type);
        }
    }
}

template <typename T>
void World::each_registered_entity_component(const entt::entity entity, T callback) const noexcept {
    for (const auto& [type, descriptor] : m_components) {
        const entt::meta_handle component_handle = get(entity, type);
        if (component_handle) {
            callback(component_handle);
        }
    }
}

template <typename T>
void World::each_editable_entity_component(const entt::entity entity, T callback) const noexcept {
    for (const auto& [type, descriptor] : m_components) {
        const entt::meta_handle component_handle = get(entity, type);
        if (component_handle && is_component_editable(type)) {
            callback(component_handle);
        }
    }
}

template <typename T>
void World::register_system(const std::string& name) noexcept {
    size_t type;
    if constexpr (std::is_base_of_v<NormalSystem, T>) {
        type = 0;
    } else {
        type = 1;
    }

    assert(m_systems[type].count(name) == 0);
    m_systems[type].emplace(name, SystemDescriptor{ [](World& world) -> std::unique_ptr<System> {
        return std::make_unique<T>(world);
    }, nullptr });
}

} // namespace hg
