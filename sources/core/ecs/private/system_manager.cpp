#include "core/ecs/system_manager.h"
#include "core/ecs/tags.h"

#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp>

#include <algorithm>

namespace hg {

std::vector<SystemManager::SystemDescriptor> SystemManager::m_systems[2];

void SystemManager::commit() {
    size_t current_tag_index = 0;

    for (std::vector<SystemDescriptor>& systems : m_systems) {
        std::unordered_map<std::string, size_t> system_mapping;

        std::sort(systems.begin(), systems.end(), [](const SystemDescriptor& lhs, const SystemDescriptor& rhs) {
            return lhs.name < rhs.name;
        });

        for (size_t i = 0; i < systems.size(); i++) {
            SystemDescriptor& system_descriptor = systems[i];

            entt::meta_prop tags_property = system_descriptor.system_type.prop("tags"_hs);
            if (tags_property) {
                entt::meta_any tags_property_value = tags_property.value();

                assert(tags_property_value);
                assert(tags_property_value.type() == entt::resolve<std::shared_ptr<TagWrapper>>());

                const auto& tags_expression = tags_property_value.fast_cast<std::shared_ptr<TagWrapper>>();
                system_descriptor.tag_expression = tags_expression.get();
            }

            assert(!system_descriptor.name.empty());
            assert(system_mapping.count(system_descriptor.name) == 0);
            system_mapping.emplace(system_descriptor.name, i);
        }

        for (size_t i = 0; i < systems.size(); i++) {
            SystemDescriptor& system_descriptor = systems[i];

            entt::meta_prop before_property = system_descriptor.system_type.prop("before"_hs);
            if (before_property) {
                entt::meta_any before_property_value = before_property.value();

                assert(before_property_value);
                assert(before_property_value.type() == entt::resolve<std::vector<const char*>>());

                const auto& systems_before = before_property_value.fast_cast<std::vector<const char*>>();
                for (const char* system_name : systems_before) {
                    assert(system_mapping.count(system_name) == 1);
                    systems[system_mapping[system_name]].after.push_back(i);
                }
            }

            entt::meta_prop after_property = system_descriptor.system_type.prop("after"_hs);
            if (after_property) {
                entt::meta_any after_property_value = after_property.value();

                assert(after_property_value);
                assert(after_property_value.type() == entt::resolve<std::vector<const char*>>());

                const auto& systems_after = after_property_value.fast_cast<std::vector<const char*>>();
                for (const char* system_name : systems_after) {
                    assert(system_mapping.count(system_name) == 1);
                    system_descriptor.after.push_back(system_mapping[system_name]);
                }
            }
        }

        for (size_t i = 0; i < systems.size(); i++) {
            SystemDescriptor& system_descriptor = systems[i];

            std::vector<size_t>& after = system_descriptor.after;
            std::sort(after.begin(), after.end());
            after.erase(std::unique(after.begin(), after.end()), after.end());

#ifndef NDEBUG
            for (size_t preceding_system : after) {
                assert(i != preceding_system);
            }
#endif
        }
    }
}

} // namespace hg
