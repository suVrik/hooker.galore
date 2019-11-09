#include "core/ecs/system_manager.h"

#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp>

namespace hg {

std::vector<SystemManager::SystemDescriptor> SystemManager::m_systems[2];
std::unordered_map<std::string, size_t> SystemManager::m_tags_mapping;

void SystemManager::commit() noexcept {
    size_t current_tag_index = 0;

    for (std::vector<SystemDescriptor>& systems : m_systems) {
        std::unordered_map<std::string, size_t> system_mapping;

        std::sort(systems.begin(), systems.end(), [](const SystemDescriptor& lhs, const SystemDescriptor& rhs) {
            return lhs.name < rhs.name;
        });

        for (size_t i = 0; i < systems.size(); i++) {
            SystemDescriptor& system_descriptor = systems[i];

            const entt::meta_prop require_property = system_descriptor.system_type.prop("require"_hs);
            if (require_property) {
                const entt::meta_any require_property_value = require_property.value();

                assert(require_property_value);
                assert(require_property_value.type() == entt::resolve<std::vector<const char*>>());

                const std::vector<const char*>& required_tags = require_property_value.fast_cast<std::vector<const char*>>();
                for (const char* const tag : required_tags) {
                    if (auto it = m_tags_mapping.find(tag); it != m_tags_mapping.end()) {
                        system_descriptor.require.push_back(it->second);
                    } else {
                        m_tags_mapping.emplace(tag, current_tag_index);
                        system_descriptor.require.push_back(current_tag_index);
                        current_tag_index++;
                    }
                }
            }

            const entt::meta_prop exclusive_property = system_descriptor.system_type.prop("exclusive"_hs);
            if (exclusive_property) {
                const entt::meta_any exclusive_property_value = exclusive_property.value();

                assert(exclusive_property_value);
                assert(exclusive_property_value.type() == entt::resolve<std::vector<const char*>>());

                const std::vector<const char*>& exclusive_tags = exclusive_property_value.fast_cast<std::vector<const char*>>();
                for (const char* const tag : exclusive_tags) {
                    if (auto it = m_tags_mapping.find(tag); it != m_tags_mapping.end()) {
                        system_descriptor.exclusive.push_back(it->second);
                    } else {
                        m_tags_mapping.emplace(tag, current_tag_index);
                        system_descriptor.exclusive.push_back(current_tag_index);
                        current_tag_index++;
                    }
                }
            }

            assert(!system_descriptor.name.empty());
            assert(system_mapping.count(system_descriptor.name) == 0);
            system_mapping.emplace(system_descriptor.name, i);
        }

        for (size_t i = 0; i < systems.size(); i++) {
            SystemDescriptor& system_descriptor = systems[i];

            const entt::meta_prop before_property = system_descriptor.system_type.prop("before"_hs);
            if (before_property) {
                const entt::meta_any before_property_value = before_property.value();

                assert(before_property_value);
                assert(before_property_value.type() == entt::resolve<std::vector<const char*>>());

                const std::vector<const char*>& systems_before = before_property_value.fast_cast<std::vector<const char*>>();
                for (const char* const system_name : systems_before) {
                    assert(system_mapping.count(system_name) == 1);
                    systems[system_mapping[system_name]].after.push_back(i);
                }
            }

            const entt::meta_prop after_property = system_descriptor.system_type.prop("after"_hs);
            if (after_property) {
                const entt::meta_any after_property_value = after_property.value();

                assert(after_property_value);
                assert(after_property_value.type() == entt::resolve<std::vector<const char*>>());

                const std::vector<const char*>& systems_after = after_property_value.fast_cast<std::vector<const char*>>();
                for (const char* const system_name : systems_after) {
                    assert(system_mapping.count(system_name) == 1);
                    system_descriptor.after.push_back(system_mapping[system_name]);
                }
            }
        }

        for (size_t i = 0; i < systems.size(); i++) {
            SystemDescriptor& system_descriptor = systems[i];

            std::vector<size_t>& require = system_descriptor.require;
            std::sort(require.begin(), require.end());
            require.erase(std::unique(require.begin(), require.end()), require.end());

            std::vector<size_t>& exclusive = system_descriptor.exclusive;
            std::sort(exclusive.begin(), exclusive.end());
            exclusive.erase(std::unique(exclusive.begin(), exclusive.end()), exclusive.end());

            std::vector<size_t>& after = system_descriptor.after;
            std::sort(after.begin(), after.end());
            after.erase(std::unique(after.begin(), after.end()), after.end());

#ifndef NDEBUG
            for (const size_t preceding_system : after) {
                assert(i != preceding_system);
            }

            for (const size_t required_tag : require) {
                assert(std::find(exclusive.begin(), exclusive.end(), required_tag) == exclusive.end());
            }
#endif
        }
    }
}

} // namespace hg
