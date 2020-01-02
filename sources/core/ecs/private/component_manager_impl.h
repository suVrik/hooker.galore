#pragma once

#include "core/ecs/component_manager.h"

namespace hg {

template <typename T>
void ComponentManager::register_component() {
    ComponentDescriptor descriptor{};

    if constexpr (std::is_default_constructible_v<T>) {
        descriptor.construct = []() -> entt::meta_any {
            return entt::meta_any(std::in_place_type<T>);
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

    if constexpr (std::is_default_constructible_v<T>) {
        descriptor.set = [](entt::registry* registry) -> entt::meta_handle {
            return entt::meta_handle(registry->set<T>());
        };
    } else {
        descriptor.set = nullptr;
    }

    descriptor.unset = [](entt::registry* registry) {
        registry->unset<T>();
    };

    descriptor.ctx = [](const entt::registry* registry) -> entt::meta_handle {
        return entt::meta_handle(*const_cast<entt::registry*>(registry)->try_ctx<T>());
    };

    // All non-single components must be either copy assignable or move assignable, because they're stored in vector-like structure.
    if constexpr (std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>) {
        if constexpr (std::is_default_constructible_v<T>) {
            descriptor.assign_default = [](entt::registry* registry, entt::entity entity) -> entt::meta_handle {
                return entt::meta_handle(registry->assign<T>(entity));
            };
        } else {
            descriptor.assign_default = nullptr;
        }

        if constexpr (std::is_copy_constructible_v<T>) {
            descriptor.assign_copy = [](entt::registry* registry, entt::entity entity, entt::meta_handle component) -> entt::meta_handle {
                return entt::meta_handle(registry->assign<T>(entity, *component.data<T>()));
            };
        } else {
            descriptor.assign_copy = nullptr;
        }

        if constexpr (std::is_move_constructible_v<T>) {
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
            descriptor.get_or_assign = [](entt::registry* registry, entt::entity entity) -> entt::meta_handle {
                return entt::meta_handle(registry->get_or_assign<T>(entity));
            };
        } else {
            descriptor.get_or_assign = nullptr;
        }
    } else {
        descriptor.assign_default = nullptr;
        descriptor.assign_copy = nullptr;
        descriptor.assign_move = nullptr;
        descriptor.replace_copy = nullptr;
        descriptor.replace_move = nullptr;
        descriptor.remove = nullptr;
        descriptor.has = nullptr;
        descriptor.get = nullptr;
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
