#pragma once

#include "core/ecs/system_manager.h"

namespace hg {

template <typename T>
void SystemManager::register_system(const std::string& name) noexcept {
    size_t type;
    if constexpr (std::is_base_of_v<NormalSystem, T>) {
        type = 0;
    } else {
        type = 1;
    }

    m_systems[type].push_back(SystemDescriptor{ [](World& world) -> std::unique_ptr<System> {
        return std::make_unique<T>(world);
    }, entt::resolve<T>(), name });
}

} // namespace hg
