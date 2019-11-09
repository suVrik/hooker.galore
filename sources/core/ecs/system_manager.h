#pragma once

#include "core/ecs/system.h"

#include <entt/meta/factory.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace hg {

class World;

/** `SystemManager` contains information about systems and everything related to them. */
class SystemManager final {
public:
    SystemManager() = delete;

    /** Register system `T` with member function `update` and specified `name`. */
    template <typename T>
    static void register_system(const std::string& name) noexcept;

    /** Link all the registered systems among themselves. */
    static void commit() noexcept;

private:
    struct SystemDescriptor final {
        std::unique_ptr<System>(*construct)(World& world);
        entt::meta_type system_type;
        std::string name;

        std::vector<size_t> require;
        std::vector<size_t> exclusive;
        std::vector<size_t> after;
    };

    static std::vector<SystemDescriptor> m_systems[2];
    static std::unordered_map<std::string, size_t> m_tags_mapping;

    friend class World;
};

} // namespace hg

#include "core/ecs/private/system_manager_impl.h"
