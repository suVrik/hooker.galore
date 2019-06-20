#pragma once

#include "core/ecs/world.h"

namespace hg {

void register_components(World& world) noexcept;
void register_systems(World& world) noexcept;

template <typename T>
void World::each(entt::entity entity, T callback) const noexcept {
    for (const auto& [type, descriptor] : m_components) {
        const entt::meta_handle component_handle = get(entity, type);
        if (component_handle) {
            callback(component_handle);
        }
    }
}

template <typename T>
void World::register_component() noexcept {
    ComponentDescriptor descriptor{};

    descriptor.assign = [](World* world, entt::entity entity) -> entt::meta_handle {
        return entt::meta_handle(world->assign<T>(entity));
    };

    descriptor.assign_copy = [](World* world, entt::entity entity, const void* copy) -> entt::meta_handle {
        if constexpr (std::is_copy_constructible_v<T>) {
            return entt::meta_handle(world->assign<T>(entity, *static_cast<const T*>(copy)));
        } else {
            assert(false && "Specified type is not copy constructible!");
            return entt::meta_handle();
        }
    };

    descriptor.remove = [](World* world, entt::entity entity) {
        world->remove<T>(entity);
    };

    descriptor.has = [](const World* world, entt::entity entity) -> bool {
        return world->has<T>(entity);
    };

    descriptor.get = [](const World* world, entt::entity entity) -> entt::meta_handle {
        return entt::meta_handle(*const_cast<World*>(world)->try_get<T>(entity));
    };

    descriptor.get_or_assign = [](World* world, entt::entity entity) -> entt::meta_handle {
        return entt::meta_handle(world->get_or_assign<T>(entity));
    };

    m_components.emplace(entt::resolve<T>(), descriptor);
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
