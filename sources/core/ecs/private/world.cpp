#include "core/ecs/world.h"

#include <chrono>

namespace hg {

World::World() {
    set<RunningWorldSingleComponent>();
    register_components(*this);
    register_systems(*this);
    commit_registered_systems();
}

World::~World() {
    for (int32_t i = 1; i >= 0; i--) {
        for (auto it = m_system_order[i].rbegin(); it != m_system_order[i].rend(); ++it) {
            const size_t system_index = *it;
            assert(system_index < m_systems[i].size());
            m_systems[i][system_index].system = nullptr;
        }
    }
}

entt::meta_any World::construct_component(const entt::meta_type component_type) const noexcept {
    assert(is_component_registered(component_type));
    assert(is_default_constructible(component_type));
    return m_components.find(component_type)->second.construct();
}

entt::meta_any World::copy_component(const entt::meta_handle component) const noexcept {
    assert(is_component_registered(component.type()));
    assert(is_copy_constructible(component.type()));
    return m_components.find(component.type())->second.copy(component);
}

entt::meta_any World::move_component(const entt::meta_handle component) const noexcept {
    assert(is_component_registered(component.type()));
    assert(is_move_constructible(component.type()));
    return m_components.find(component.type())->second.move(component);
}

const char* World::get_component_name(const entt::meta_type component_type, const char* const fallback) const noexcept {
    assert(is_component_registered(component_type));

    const entt::meta_prop component_name_property = component_type.prop("name"_hs);
    if (component_name_property && component_name_property.value().type() == entt::resolve<const char*>()) {
        return component_name_property.value().cast<const char*>();
    }
    return fallback;
}

bool World::is_component_ignored(const entt::meta_type component_type) const noexcept {
    assert(is_component_registered(component_type));

    const entt::meta_prop ignore_property = component_type.prop("ignore"_hs);
    return ignore_property && ignore_property.value().type() == entt::resolve<bool>() && ignore_property.value().cast<bool>();
}

bool World::is_component_editable(const entt::meta_type component_type) const noexcept {
    assert(is_component_registered(component_type));

    const entt::meta_prop component_name_property = component_type.prop("name"_hs);
    if (component_name_property && component_name_property.value().type() == entt::resolve<const char*>()) {
        const entt::meta_prop ignore_property = component_type.prop("ignore"_hs);
        if (!ignore_property || ignore_property.value().type() != entt::resolve<bool>() || !ignore_property.value().cast<bool>()) {
            return is_default_constructible(component_type) && is_copy_constructible(component_type) && is_copy_assignable(component_type);
        }
    }
    return false;
}

bool World::is_component_registered(const entt::meta_type component_type) const noexcept {
    return m_components.count(component_type) != 0;
}

entt::meta_handle World::assign_default(const entt::entity entity, const entt::meta_type component_type) noexcept {
    assert(is_component_registered(component_type));
    assert(is_default_constructible(component_type));
    return m_components.find(component_type)->second.assign_default(this, entity);
}

entt::meta_handle World::assign_copy(const entt::entity entity, const entt::meta_handle component) noexcept {
    assert(is_component_registered(component.type()));
    assert(is_copy_constructible(component.type()));
    return m_components.find(component.type())->second.assign_copy(this, entity, component);
}

entt::meta_handle World::assign_move(entt::entity entity, const entt::meta_handle component) noexcept {
    assert(is_component_registered(component.type()));
    assert(is_move_constructible(component.type()));
    return m_components.find(component.type())->second.assign_move(this, entity, component);
}

entt::meta_handle World::replace_copy(const entt::entity entity, const entt::meta_handle component) noexcept {
    assert(is_component_registered(component.type()));
    assert(is_copy_assignable(component.type()));
    return m_components.find(component.type())->second.replace_copy(this, entity, component);
}

entt::meta_handle World::replace_move(const entt::entity entity, const entt::meta_handle component) noexcept {
    assert(is_component_registered(component.type()));
    assert(is_move_assignable(component.type()));
    return m_components.find(component.type())->second.replace_move(this, entity, component);
}

void World::remove(const entt::entity entity, const entt::meta_type component_type) noexcept {
    assert(is_component_registered(component_type));
    m_components.find(component_type)->second.remove(this, entity);
}

bool World::has(const entt::entity entity, const entt::meta_type component_type) const noexcept {
    assert(is_component_registered(component_type));
    return m_components.find(component_type)->second.has(this, entity);
}

entt::meta_handle World::get(const entt::entity entity, const entt::meta_type component_type) const noexcept {
    assert(is_component_registered(component_type));
    return m_components.find(component_type)->second.get(this, entity);
}

entt::meta_handle World::get_or_assign(const entt::entity entity, const entt::meta_type component_type) noexcept {
    assert(is_component_registered(component_type));
    assert(is_default_constructible(component_type));
    return m_components.find(component_type)->second.get_or_assign(this, entity);
}

bool World::is_default_constructible(const entt::meta_type component_type) const noexcept {
    assert((m_components.find(component_type)->second.assign_default == nullptr) == (m_components.find(component_type)->second.get_or_assign == nullptr));
    return m_components.find(component_type)->second.assign_default != nullptr;
}

bool World::is_copy_constructible(const entt::meta_type component_type) const noexcept {
    assert((m_components.find(component_type)->second.assign_copy != nullptr) == (m_components.find(component_type)->second.copy != nullptr));
    return m_components.find(component_type)->second.assign_copy != nullptr;
}

bool World::is_move_constructible(const entt::meta_type component_type) const noexcept {
    assert((m_components.find(component_type)->second.assign_move != nullptr) == (m_components.find(component_type)->second.move != nullptr));
    return m_components.find(component_type)->second.assign_move != nullptr;
}

bool World::is_copy_assignable(const entt::meta_type component_type) const noexcept {
    return m_components.find(component_type)->second.replace_copy != nullptr;
}

bool World::is_move_assignable(const entt::meta_type component_type) const noexcept {
    return m_components.find(component_type)->second.replace_move != nullptr;
}

void World::clear_tags() noexcept {
    for (uint8_t& tag_enabled : m_tags) {
        if (tag_enabled != 0) {
            tag_enabled = false;
            m_tags_changed[0] = m_tags_changed[1] = true;
        }
    }
}

void World::add_tag(const char* const tag) noexcept {
    if (auto it = m_tags_mapping.find(tag); it != m_tags_mapping.end()) {
        assert(it->second < m_tags.size());
        if (!m_tags[it->second]) {
            m_tags[it->second] = true;
            m_tags_changed[0] = m_tags_changed[1] = true;
        }
    }
}

void World::remove_tag(const char* const tag) noexcept {
    if (auto it = m_tags_mapping.find(tag); it != m_tags_mapping.end()) {
        assert(it->second < m_tags.size());
        if (m_tags[it->second]) {
            m_tags[it->second] = false;
            m_tags_changed[0] = m_tags_changed[1] = true;
        }
    }
}

bool World::check_tag(const char* const tag) noexcept {
    if (auto it = m_tags_mapping.find(tag); it != m_tags_mapping.end()) {
        assert(it->second < m_tags.size());
        return m_tags[it->second];
    }
    return false;
}

bool World::update_normal(const float elapsed_time) noexcept {
    if (m_tags_changed[0]) {
        sort_systems(0);
        m_tags_changed[0] = false;
    }

    for (const size_t system_index : m_system_order[0]) {
        assert(system_index < m_systems[0].size());
        assert(m_systems[0][system_index].system);

        if (try_ctx<RunningWorldSingleComponent>() == nullptr) {
            return false;
        }
        
        m_systems[0][system_index].system->update(elapsed_time);
    }
    return true;
}

void World::update_fixed(const float elapsed_time) noexcept {
    if (m_tags_changed[1]) {
        sort_systems(1);
        m_tags_changed[1] = false;
    }

    for (const size_t system_index : m_system_order[1]) {
        assert(system_index < m_systems[1].size());
        assert(m_systems[1][system_index].system);

        m_systems[1][system_index].system->update(elapsed_time);
    }
}

void World::commit_registered_systems() noexcept {
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
                        m_tags_mapping.emplace(tag, m_tags.size());
                        system_descriptor.require.push_back(m_tags.size());
                        m_tags.push_back(false);
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

            std::vector<size_t>& after = system_descriptor.after;
            std::sort(after.begin(), after.end());
            after.erase(std::unique(after.begin(), after.end()), after.end());

#ifndef NDEBUG
            for (const size_t preceding_system : after) {
                assert(i != preceding_system);
            }
#endif
        }
    }
}

void World::sort_systems(const size_t system_type) noexcept {
    [[maybe_unused]] const std::chrono::steady_clock::time_point before_sort = std::chrono::steady_clock::now();

    assert(system_type < 2);

    std::vector<SystemDescriptor>& systems = m_systems[system_type];
    std::vector<PropagateState>& propagate_state = m_propagate_state[system_type];
    std::vector<size_t>& system_order = m_system_order[system_type];

    propagate_state.assign(systems.size(), PropagateState::NOT_VISITED);

    const std::vector<size_t> old_system_order = system_order;
    system_order.clear();

    for (size_t i = 0; i < systems.size(); i++) {
        assert(propagate_state[i] != PropagateState::IN_PROGRESS);
        if (propagate_state[i] == PropagateState::NOT_VISITED) {
            if (check_tag_indices(systems[i].require)) {
                // For system order debugging.
                [[maybe_unused]] const std::string& system_name = systems[i].name;

                propagate_system(system_type, i);
                assert(propagate_state[i] == PropagateState::COMPLETED);
            }
        }
    }

    for (auto it = old_system_order.rbegin(); it != old_system_order.rend(); ++it) {
        const size_t system_index = *it;

        assert(system_index < propagate_state.size());
        assert(propagate_state[system_index] != PropagateState::IN_PROGRESS);
        if (propagate_state[system_index] == PropagateState::NOT_VISITED) {
            assert(system_index < systems.size());
            if (systems[system_index].system) {
                systems[system_index].system = nullptr;
            }
        }
    }

    [[maybe_unused]] const std::chrono::steady_clock::time_point before_constructors = std::chrono::steady_clock::now();

    for (int i = 0; i < system_order.size(); i++) {
        SystemDescriptor& system = systems[system_order[i]];
        if (!system.system) {
            const std::chrono::steady_clock::time_point before_constructor = std::chrono::steady_clock::now();

            system.system = system.construct(*this);
            assert(system.system);

            const std::chrono::steady_clock::time_point after_constructor = std::chrono::steady_clock::now();
            system.construction_duration = std::chrono::duration<float>(after_constructor - before_constructor).count();
        } else {
            system.construction_duration = 0.f;
        }
    }

#ifndef NDEBUG
    const std::chrono::steady_clock::time_point after_sort = std::chrono::steady_clock::now();
    const float total_duration = std::chrono::duration<float>(after_sort - before_sort).count();
    const float constructors_duration = std::chrono::duration<float>(after_sort - before_constructors).count();

    if (system_type == 0) {
        printf("[ORDER] Normal systems had been reordered:\n");
    } else {
        printf("[ORDER] Fixed systems had been reordered:\n");
    }
    for (int i = 0; i < system_order.size(); i++) {
        SystemDescriptor& system = systems[system_order[i]];
        if (std::abs(system.construction_duration) != 0.f && constructors_duration > 1e-5f) {
            printf("[ORDER] %3d %.55s (%.1f%%)\n", i + 1, system.name.c_str(), system.construction_duration / constructors_duration * 100.f);
        } else {
            printf("[ORDER] %3d %.65s\n", i + 1, system.name.c_str());
        }
    }
    printf("[ORDER] Reordering took %.3f seconds.\n", std::chrono::duration<float>(after_sort - before_sort).count());
#endif
}

bool World::check_tag_indices(const std::vector<size_t>& tags) noexcept {
    for (const size_t required_tag : tags) {
        assert(required_tag < m_tags.size());
        if (!m_tags[required_tag]) {
            return false;
        }
    }
    return true;
}

void World::propagate_system(const size_t system_type, const size_t system_index) noexcept {
    assert(system_type < 2);

    std::vector<SystemDescriptor>& systems = m_systems[system_type];
    assert(system_index < systems.size());

    SystemDescriptor& system = systems[system_index];
    assert(check_tag_indices(system.require));

    // For system order debugging.
    [[maybe_unused]] const std::string& following_system_name = system.name;

    std::vector<PropagateState>& propagate_state = m_propagate_state[system_type];
    assert(system_index < propagate_state.size());
    assert(propagate_state[system_index] == PropagateState::NOT_VISITED);

    propagate_state[system_index] = PropagateState::IN_PROGRESS;

    for (const size_t preceding_system_index : system.after) {
        assert(preceding_system_index < systems.size());
        assert(preceding_system_index < propagate_state.size());

        // For system order debugging.
        [[maybe_unused]] const std::string& preceding_system_name = systems[preceding_system_index].name;

        assert(propagate_state[preceding_system_index] != PropagateState::IN_PROGRESS && "System order loop. Please, reorder your systems manually.");
        if (propagate_state[preceding_system_index] == PropagateState::NOT_VISITED) {
            if (check_tag_indices(systems[preceding_system_index].require)) {
                propagate_system(system_type, preceding_system_index);
            }
        }
    }

    m_system_order[system_type].push_back(system_index);

    propagate_state[system_index] = PropagateState::COMPLETED;
}

} // namespace hg
