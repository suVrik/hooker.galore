#pragma once

#include "core/ecs/world.h"

namespace hg {

void register_components(World& world) noexcept;
void register_systems(World& world) noexcept;

template <typename T>
void World::register_component() noexcept {
    ComponentDescriptor descriptor{};

    descriptor.assign = [](World* world, entt::entity entity) -> entt::meta_handle {
        return entt::meta_handle(world->assign<T>(entity));
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
