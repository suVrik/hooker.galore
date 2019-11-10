#include "core/ecs/system_manager.h"
#include "core/ecs/world.h"

#include <chrono>

namespace hg {

World::World(World* const parent)
        : m_parent(parent) {
    if (m_parent != nullptr) {
        m_parent->m_children.push_back(this);
    }

    set<RunningWorldSingleComponent>();

    for (size_t i = 0; i < std::size(m_systems); i++) {
        m_systems[i].resize(SystemManager::m_systems[i].size());
    }

    m_tags.assign(SystemManager::m_tags_mapping.size(), 0);
}

World::~World() {
    if (m_parent != nullptr) {
        auto it = std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this);
        assert(it != m_parent->m_children.end());
        std::swap(*it, m_parent->m_children.back());
        m_parent->m_children.pop_back();
    }

    for (int32_t i = 1; i >= 0; i--) {
        for (auto it = m_system_order[i].rbegin(); it != m_system_order[i].rend(); ++it) {
            const size_t system_index = *it;
            assert(system_index < m_systems[i].size());
            m_systems[i][system_index].instance = nullptr;
        }
    }
}

World* World::get_parent() const {
    return m_parent;
}

//////////////////////////////////////////////////////////////////////////

entt::meta_handle World::ctx(entt::meta_type single_component_type) const {
    assert(ComponentManager::is_registered(single_component_type));
    const entt::meta_handle result = ComponentManager::descriptors[single_component_type].ctx(this);
    if (!result && m_parent != nullptr) {
        return ComponentManager::descriptors[single_component_type].ctx(m_parent);
    }
    return result;
}

bool World::has_ctx(entt::meta_type single_component_type) const {
    return static_cast<bool>(ctx(single_component_type));
}

bool World::is_owned_ctx(entt::meta_type single_component_type) const {
    return m_parent == nullptr || ctx(single_component_type) != m_parent->ctx(single_component_type);
}

//////////////////////////////////////////////////////////////////////////

entt::meta_handle World::assign_default(const entt::entity entity, const entt::meta_type component_type) {
    assert(ComponentManager::is_registered(component_type));
    assert(ComponentManager::is_default_constructible(component_type));
    return ComponentManager::descriptors[component_type].assign_default(this, entity);
}

entt::meta_handle World::assign_copy(const entt::entity entity, const entt::meta_handle component) {
    assert(ComponentManager::is_registered(component.type()));
    assert(ComponentManager::is_copy_constructible(component.type()));
    return ComponentManager::descriptors[component.type()].assign_copy(this, entity, component);
}

entt::meta_handle World::assign_move(entt::entity entity, const entt::meta_handle component) {
    assert(ComponentManager::is_registered(component.type()));
    assert(ComponentManager::is_move_constructible(component.type()));
    return ComponentManager::descriptors[component.type()].assign_move(this, entity, component);
}

entt::meta_handle World::assign_move_or_copy(entt::entity entity, entt::meta_handle component) {
    if (ComponentManager::is_move_constructible(component.type())) {
        return assign_move(entity, component);
    }
    return assign_copy(entity, component);
}

entt::meta_handle World::replace_copy(const entt::entity entity, const entt::meta_handle component) {
    assert(ComponentManager::is_registered(component.type()));
    assert(ComponentManager::is_copy_assignable(component.type()));
    return ComponentManager::descriptors[component.type()].replace_copy(this, entity, component);
}

entt::meta_handle World::replace_move(const entt::entity entity, const entt::meta_handle component) {
    assert(ComponentManager::is_registered(component.type()));
    assert(ComponentManager::is_move_assignable(component.type()));
    return ComponentManager::descriptors[component.type()].replace_move(this, entity, component);
}

entt::meta_handle World::replace_move_or_copy(const entt::entity entity, const entt::meta_handle component) {
    if (ComponentManager::is_move_assignable(component.type())) {
        return replace_move(entity, component);
    }
    return replace_copy(entity, component);
}

void World::remove(const entt::entity entity, const entt::meta_type component_type) {
    assert(ComponentManager::is_registered(component_type));
    ComponentManager::descriptors[component_type].remove(this, entity);
}

bool World::has(const entt::entity entity, const entt::meta_type component_type) const {
    assert(ComponentManager::is_registered(component_type));
    return ComponentManager::descriptors[component_type].has(this, entity);
}

entt::meta_handle World::get(const entt::entity entity, const entt::meta_type component_type) const {
    assert(ComponentManager::is_registered(component_type));
    return ComponentManager::descriptors[component_type].get(this, entity);
}

entt::meta_handle World::get_or_assign(const entt::entity entity, const entt::meta_type component_type) {
    assert(ComponentManager::is_registered(component_type));
    assert(ComponentManager::is_default_constructible(component_type));
    return ComponentManager::descriptors[component_type].get_or_assign(this, entity);
}

//////////////////////////////////////////////////////////////////////////

void World::clear_tags() {
    for (uint8_t& tag_enabled : m_tags) {
        if (tag_enabled != 0) {
            tag_enabled = false;
            m_tags_changed[0] = m_tags_changed[1] = true;
        }
    }
}

void World::add_tag(const char* const tag) {
    if (auto it = SystemManager::m_tags_mapping.find(tag); it != SystemManager::m_tags_mapping.end()) {
        assert(it->second < m_tags.size());
        if (!m_tags[it->second]) {
            m_tags[it->second] = true;
            m_tags_changed[0] = m_tags_changed[1] = true;
        }
    }
}

void World::remove_tag(const char* const tag) {
    if (auto it = SystemManager::m_tags_mapping.find(tag); it != SystemManager::m_tags_mapping.end()) {
        assert(it->second < m_tags.size());
        if (m_tags[it->second]) {
            m_tags[it->second] = false;
            m_tags_changed[0] = m_tags_changed[1] = true;
        }
    }
}

bool World::check_tag(const char* const tag) {
    if (auto it = SystemManager::m_tags_mapping.find(tag); it != SystemManager::m_tags_mapping.end()) {
        assert(it->second < m_tags.size());
        return m_tags[it->second];
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

bool World::update_normal(const float elapsed_time) {
    if (m_tags_changed[0]) {
        sort_systems(0);
        m_tags_changed[0] = false;
    }

    for (const size_t system_index : m_system_order[0]) {
        assert(system_index < m_systems[0].size());
        assert(m_systems[0][system_index].instance);

        if (try_ctx<RunningWorldSingleComponent>() == nullptr) {
            return false;
        }
        
        m_systems[0][system_index].instance->update(elapsed_time);
    }
    return true;
}

void World::update_fixed(const float elapsed_time) {
    if (m_tags_changed[1]) {
        sort_systems(1);
        m_tags_changed[1] = false;
    }

    for (const size_t system_index : m_system_order[1]) {
        assert(system_index < m_systems[1].size());
        assert(m_systems[1][system_index].instance);

        m_systems[1][system_index].instance->update(elapsed_time);
    }
}

void World::sort_systems(const size_t system_type) {
    [[maybe_unused]] const std::chrono::steady_clock::time_point before_sort = std::chrono::steady_clock::now();

    assert(system_type < 2);

    std::vector<SystemManager::SystemDescriptor>& system_descriptors = SystemManager::m_systems[system_type];
    std::vector<SystemInstance>& system_instances = m_systems[system_type];
    assert(system_descriptors.size() == system_instances.size());

    std::vector<PropagateState>& propagate_state = m_propagate_state[system_type];
    std::vector<size_t>& system_order = m_system_order[system_type];

    propagate_state.assign(system_descriptors.size(), PropagateState::NOT_VISITED);

    const std::vector<size_t> old_system_order = system_order;
    system_order.clear();

    for (size_t i = 0; i < system_descriptors.size(); i++) {
        assert(propagate_state[i] != PropagateState::IN_PROGRESS);
        if (propagate_state[i] == PropagateState::NOT_VISITED) {
            if (check_tag_indices(system_descriptors[i].require, system_descriptors[i].exclusive)) {
                // For system order debugging.
                [[maybe_unused]] const std::string& system_name = system_descriptors[i].name;

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
            assert(system_index < system_instances.size());
            if (system_instances[system_index].instance) {
                system_instances[system_index].instance = nullptr;
            }
        }
    }

    [[maybe_unused]] const std::chrono::steady_clock::time_point before_constructors = std::chrono::steady_clock::now();

    for (int i = 0; i < system_order.size(); i++) {
        SystemManager::SystemDescriptor& system_descriptor = system_descriptors[system_order[i]];
        SystemInstance& system_instance = system_instances[system_order[i]];
        if (!system_instance.instance) {
            const std::chrono::steady_clock::time_point before_constructor = std::chrono::steady_clock::now();

            system_instance.instance = system_descriptor.construct(*this);
            assert(system_instance.instance);

            const std::chrono::steady_clock::time_point after_constructor = std::chrono::steady_clock::now();
            system_instance.construction_duration = std::chrono::duration<float>(after_constructor - before_constructor).count();
        } else {
            system_instance.construction_duration = 0.f;
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
        SystemManager::SystemDescriptor& system_descriptor = system_descriptors[system_order[i]];
        SystemInstance& system_instance = system_instances[system_order[i]];
        if (system_instance.construction_duration > 1e-5f && constructors_duration > 1e-5f) {
            printf("[ORDER] %3d %.55s (%.1f%%)\n", i + 1, system_descriptor.name.c_str(), system_instance.construction_duration / constructors_duration * 100.0);
        } else {
            printf("[ORDER] %3d %.65s\n", i + 1, system_descriptor.name.c_str());
        }
    }
    printf("[ORDER] Reordering took %.3f seconds.\n", std::chrono::duration<float>(after_sort - before_sort).count());
#endif
}

bool World::check_tag_indices(const std::vector<size_t>& require, const std::vector<size_t>& exclusive) {
    for (const size_t required_tag : require) {
        assert(required_tag < m_tags.size());
        if (!m_tags[required_tag]) {
            return false;
        }
    }
    for (const size_t exclusive_tag : exclusive) {
        assert(exclusive_tag < m_tags.size());
        if (m_tags[exclusive_tag]) {
            return false;
        }
    }
    return true;
}

void World::propagate_system(const size_t system_type, const size_t system_index) {
    assert(system_type < 2);

    std::vector<SystemManager::SystemDescriptor>& systems = SystemManager::m_systems[system_type];
    assert(system_index < systems.size());

    SystemManager::SystemDescriptor& system = systems[system_index];
    assert(check_tag_indices(system.require, system.exclusive));

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
            if (check_tag_indices(systems[preceding_system_index].require, systems[preceding_system_index].exclusive)) {
                propagate_system(system_type, preceding_system_index);
            }
        }
    }

    m_system_order[system_type].push_back(system_index);

    propagate_state[system_index] = PropagateState::COMPLETED;
}

} // namespace hg
