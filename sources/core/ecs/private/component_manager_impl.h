#pragma once

#include "core/ecs/component_manager.h"

namespace hg {

template <typename T>
void ComponentManager::register_component() {
    ComponentDescriptor descriptor{};

    if constexpr (std::is_default_constructible_v<T>) {
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
        descriptor.move = [](entt::meta_handle component) -> entt::meta_any {
            return entt::meta_any(T(std::move(*component.data<T>())));
        };
    } else {
        descriptor.move = nullptr;
    }

    descriptor.ctx = [](const entt::registry* registry) -> entt::meta_handle {
        if constexpr (std::is_empty_v<T>) {
            static T instance;
            return entt::meta_handle(instance);
        } else {
            return entt::meta_handle(*const_cast<entt::registry*>(registry)->try_ctx<T>());
        }
    };

    if constexpr (std::is_default_constructible_v<T>) {
        static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>, "EnTT requires component type to be either copy or move assignable as well.");
        descriptor.assign_default = [](entt::registry* registry, entt::entity entity) -> entt::meta_handle {
            return entt::meta_handle(registry->assign<T>(entity));
        };
    } else {
        descriptor.assign_default = nullptr;
    }

    if constexpr (std::is_copy_constructible_v<T>) {
        static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>, "EnTT requires component type to be either copy or move assignable as well.");
        descriptor.assign_copy = [](entt::registry* registry, entt::entity entity, entt::meta_handle component) -> entt::meta_handle {
            return entt::meta_handle(registry->assign<T>(entity, *component.data<T>()));
        };
    } else {
        descriptor.assign_copy = nullptr;
    }

    if constexpr (std::is_move_constructible_v<T>) {
        static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>, "EnTT requires component type to be either copy or move assignable as well.");
        descriptor.assign_move = [](entt::registry* registry, entt::entity entity, entt::meta_handle component) -> entt::meta_handle {
            return entt::meta_handle(registry->assign<T>(entity, std::move(*component.data<T>())));
        };
    } else {
        descriptor.assign_move = nullptr;
    }

    if constexpr (std::is_copy_assignable_v<T>) {
        descriptor.replace_copy = [](entt::registry* registry, entt::entity entity, entt::meta_handle component) -> entt::meta_handle {
            return entt::meta_handle(registry->replace<T>(entity, *component.data<T>()));
        };
    } else {
        descriptor.replace_copy = nullptr;
    }

    if constexpr (std::is_move_assignable_v<T>) {
        descriptor.replace_move = [](entt::registry* registry, entt::entity entity, entt::meta_handle component) -> entt::meta_handle {
            return entt::meta_handle(registry->replace<T>(entity, std::move(*component.data<T>())));
        };
    } else {
        descriptor.replace_move = nullptr;
    }

    descriptor.remove = [](entt::registry* registry, entt::entity entity) {
        registry->remove<T>(entity);
    };

    descriptor.has = [](const entt::registry* registry, entt::entity entity) -> bool {
        return registry->has<T>(entity);
    };

    descriptor.get = [](const entt::registry* registry, entt::entity entity) -> entt::meta_handle {
        if constexpr (std::is_empty_v<T>) {
            static T instance;
            return entt::meta_handle(instance);
        } else {
            return entt::meta_handle(*const_cast<entt::registry*>(registry)->try_get<T>(entity));
        }
    };

    if (std::is_default_constructible_v<T>) {
        static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>, "EnTT requires component type to be either copy or move assignable as well.");
        descriptor.get_or_assign = [](entt::registry* registry, entt::entity entity) -> entt::meta_handle {
            return entt::meta_handle(registry->get_or_assign<T>(entity));
        };
    } else {
        descriptor.get_or_assign = nullptr;
    }

    descriptors.emplace(entt::resolve<T>(), descriptor);
}

template <typename T>
void ComponentManager::each_registered(T callback) {
    for (auto& [type, descriptor] : descriptors) {
        callback(type);
    }
}

template <typename T>
void ComponentManager::each_editable(T callback) {
    for (auto& [type, descriptor] : descriptors) {
        if (is_editable(type)) {
            callback(type);
        }
    }
}

} // namespace hg
