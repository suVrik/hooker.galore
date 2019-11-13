#include "core/ecs/system_manager.h"
#include "core/ecs/world.h"

#include <chrono>

namespace hg {

World::World(World* const parent)
        : m_parent(parent) {
    if (m_parent != nullptr) {
        m_parent->m_children.push_back(this);
        
        // TODO: inherit_all_tags call or something?
        // Go in reverse order to avoid many "reserve" calls.
        for (size_t i = m_parent->m_all_tags.size(); i > 0; i--) {
            if (m_parent->m_all_tags[i - 1]) {
                const Tag tag = Tag::get_tag_by_index(i - 1);
                if (tag.is_inheritable()) {
                    add_tag(tag);
                }
            }
        }
    }

    set<RunningWorldSingleComponent>();

    for (size_t i = 0; i < std::size(m_systems); i++) {
        m_systems[i].resize(SystemManager::m_systems[i].size());
    }
}

World::~World() {
    if (m_parent != nullptr) {
        clear_tags();

        auto it = std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this);
        assert(it != m_parent->m_children.end());
        std::swap(*it, m_parent->m_children.back());
        m_parent->m_children.pop_back();
    }

    for (int32_t i = static_cast<int32_t>(std::size(m_systems) - 1); i >= 0; i--) {
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

World* World::get_root() const {
    if (m_parent != nullptr) {
        return m_parent->get_root();
    }
    return const_cast<World*>(this);
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
    for (size_t i = 0, size = m_owned_tags.size(); i < size; i++) {
        if (m_owned_tags[i]) {
            remove_tag(Tag::get_tag_by_index(i));
        }
    }
}

void World::add_tag(const Tag tag) {
    const size_t tag_index = tag.get_index();

    if (m_owned_tags.size() <= tag_index) {
        m_owned_tags.resize(tag_index + 1, false);

        if (m_all_tags.size() <= tag_index) {
            m_all_tags.resize(tag_index + 1, false);
        }
    }

    assert(tag_index < m_owned_tags.size());
    assert(tag_index < m_all_tags.size());

    if (!m_owned_tags[tag_index]) {
        m_owned_tags[tag_index] = true;

        if (!m_all_tags[tag_index]) {
            m_all_tags[tag_index] = true;
            m_tags_changed[NORMAL] = m_tags_changed[FIXED] = true;

            update_active_tags();

            if (tag.is_inheritable()) {
                for (World* const child_world : m_children) {
                    child_world->inherit_add_tag(tag);
                }
            }

            if (tag.is_propagable() && m_parent != nullptr) {
                m_parent->propagate_add_tag(this, tag);
            }
        }
    }
}

void World::remove_tag(const Tag tag) {
    const size_t tag_index = tag.get_index();
    if (tag_index < m_owned_tags.size() && m_owned_tags[tag_index]) {
        assert(tag_index < m_all_tags.size());
        assert(m_all_tags[tag_index]);

        m_owned_tags[tag_index] = false;

        const bool is_present = (tag_index < m_inherited_tags.size() && m_inherited_tags[tag_index]) ||
                                (tag_index < m_propagated_tags.size() && m_propagated_tags[tag_index] > 0);
        if (!is_present) {
            m_all_tags[tag_index] = false;
            m_tags_changed[NORMAL] = m_tags_changed[FIXED] = true;

            update_active_tags();

            if (tag.is_inheritable()) {
                for (World* const child_world : m_children) {
                    child_world->inherit_remove_tag(tag);
                }
            }

            if (tag.is_propagable() && m_parent != nullptr) {
                m_parent->propagate_remove_tag(this, tag);
            }
        }
    }
}

bool World::check_tag(const Tag tag) const {
    const size_t tag_index = tag.get_index();
    return tag_index < m_all_tags.size() && m_all_tags[tag_index];
}

bool World::check_owned_tag(const Tag tag) const {
    const size_t tag_index = tag.get_index();
    return tag_index < m_owned_tags.size() && m_owned_tags[tag_index];
}

bool World::check_active_tag(Tag tag) const {
    const size_t tag_index = tag.get_index();
    return tag_index < m_active_tags.size() && m_active_tags[tag_index];
}

void World::inherit_add_tag(const Tag tag) {
    assert(tag.is_inheritable());

    const size_t tag_index = tag.get_index();

    if (m_inherited_tags.size() <= tag_index) {
        m_inherited_tags.resize(tag_index + 1, false);

        if (m_all_tags.size() <= tag_index) {
            m_all_tags.resize(tag_index + 1, false);
        }
    }

    assert(tag_index < m_inherited_tags.size());
    assert(tag_index < m_all_tags.size());
    assert(!m_inherited_tags[tag_index]);

    m_inherited_tags[tag_index] = true;

    if (!m_all_tags[tag_index]) {
        m_all_tags[tag_index] = true;
        m_tags_changed[NORMAL] = m_tags_changed[FIXED] = true;

        update_active_tags();

        for (World* const child_world : m_children) {
            child_world->inherit_add_tag(tag);
        }
    }
}

void World::inherit_remove_tag(const Tag tag) {
    assert(tag.is_inheritable());

    const size_t tag_index = tag.get_index();

    assert(tag_index < m_inherited_tags.size());
    assert(tag_index < m_all_tags.size());
    assert(m_inherited_tags[tag_index]);
    assert(m_all_tags[tag_index]);

    m_inherited_tags[tag_index] = false;

    const bool is_present = (tag_index < m_owned_tags.size() && m_owned_tags[tag_index]) ||
                            (tag_index < m_propagated_tags.size() && m_propagated_tags[tag_index] > 0);
    if (!is_present) {
        m_all_tags[tag_index] = false;
        m_tags_changed[NORMAL] = m_tags_changed[FIXED] = true;

        update_active_tags();

        for (World* const child_world : m_children) {
            child_world->inherit_remove_tag(tag);
        }
    }
}

void World::propagate_add_tag(const World* const child_world, const Tag tag) {
    assert(tag.is_propagable());

    const size_t tag_index = tag.get_index();

    if (m_propagated_tags.size() <= tag_index) {
        m_propagated_tags.resize(tag_index + 1, 0);

        if (m_all_tags.size() <= tag_index) {
            m_all_tags.resize(tag_index + 1, false);
        }
    }

    assert(tag_index < m_inherited_tags.size());
    assert(tag_index < m_all_tags.size());

    m_propagated_tags[tag_index]++;

    if (!m_all_tags[tag_index]) {
        m_all_tags[tag_index] = true;
        m_tags_changed[NORMAL] = m_tags_changed[FIXED] = true;

        update_active_tags();

        if (tag.is_inheritable()) {
            for (World* const another_child_world : m_children) {
                if (child_world != another_child_world) {
                    another_child_world->inherit_add_tag(tag);
                }
            }
        }

        if (m_parent != nullptr) {
            m_parent->propagate_add_tag(this, tag);
        }
    }
}

void World::propagate_remove_tag(const World* const child_world, const Tag tag) {
    assert(tag.is_propagable());

    const size_t tag_index = tag.get_index();

    assert(tag_index < m_propagated_tags.size());
    assert(tag_index < m_all_tags.size());
    assert(m_propagated_tags[tag_index] > 0);
    assert(m_all_tags[tag_index]);

    m_propagated_tags[tag_index]--;

    if (m_propagated_tags[tag_index] == 0) {
        const bool is_present = (tag_index < m_owned_tags.size() && m_owned_tags[tag_index]) ||
                                (tag_index < m_inherited_tags.size() && m_inherited_tags[tag_index]);
        if (!is_present) {
            m_all_tags[tag_index] = false;
            m_tags_changed[NORMAL] = m_tags_changed[FIXED] = true;

            update_active_tags();

            if (tag.is_inheritable()) {
                for (World* const another_child_world : m_children) {
                    if (child_world != another_child_world) {
                        another_child_world->inherit_remove_tag(tag);
                    }
                }
            }

            if (m_parent != nullptr) {
                m_parent->propagate_remove_tag(this, tag);
            }
        }
    }
}

void World::update_active_tags() {
    m_active_tags.resize(m_all_tags.size(), false);
    for (size_t i = 0, size = m_active_tags.size(); i < size; i++) {
        if (m_all_tags[i]) {
            const Tag tag = Tag::get_tag_by_index(i);
            m_active_tags[i] = tag.test_requirements(m_all_tags);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

bool World::update_normal(const float elapsed_time) {
    if (m_tags_changed[NORMAL]) {
        sort_systems(NORMAL);
        m_tags_changed[NORMAL] = false;
    }

    for (const size_t system_index : m_system_order[NORMAL]) {
        assert(system_index < m_systems[NORMAL].size());
        assert(m_systems[NORMAL][system_index].instance);

        if (try_ctx<RunningWorldSingleComponent>() == nullptr) {
            return false;
        }

        m_systems[NORMAL][system_index].instance->update(elapsed_time);
    }
    return true;
}

void World::update_fixed(const float elapsed_time) {
    if (m_tags_changed[FIXED]) {
        sort_systems(FIXED);
        m_tags_changed[FIXED] = false;
    }

    for (const size_t system_index : m_system_order[FIXED]) {
        assert(system_index < m_systems[FIXED].size());
        assert(m_systems[FIXED][system_index].instance);

        m_systems[FIXED][system_index].instance->update(elapsed_time);
    }
}

void World::sort_systems(const size_t system_type) {
    [[maybe_unused]] const std::chrono::steady_clock::time_point before_sort = std::chrono::steady_clock::now();

    assert(system_type < std::size(m_systems));

    std::vector<SystemManager::SystemDescriptor>& system_descriptors = SystemManager::m_systems[system_type];
    std::vector<SystemInstance>& system_instances = m_systems[system_type];
    assert(system_descriptors.size() == system_instances.size());

    std::vector<PropagateState>& propagate_state = m_propagate_state[system_type];
    std::vector<size_t>& system_order = m_system_order[system_type];

    propagate_state.assign(system_descriptors.size(), PropagateState::NOT_VISITED);

    const std::vector<size_t> old_system_order = system_order;
    system_order.clear();

    for (size_t i = 0, size = system_descriptors.size(); i < size; i++) {
        assert(propagate_state[i] != PropagateState::IN_PROGRESS);
        if (propagate_state[i] == PropagateState::NOT_VISITED) {
            if (system_descriptors[i].tag_expression->test(m_active_tags)) {
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

    for (size_t i = 0, size = system_order.size(); i < size; i++) {
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

    if (system_type == NORMAL) {
        printf("[ORDER] Normal systems had been reordered:\n");
    } else {
        printf("[ORDER] Fixed systems had been reordered:\n");
    }
    for (size_t i = 0, size = system_order.size(); i < size; i++) {
        SystemManager::SystemDescriptor& system_descriptor = system_descriptors[system_order[i]];
        SystemInstance& system_instance = system_instances[system_order[i]];
        if (system_instance.construction_duration > 1e-5f && constructors_duration > 1e-5f) {
            printf("[ORDER] %3d %.55s (%.1f%%)\n", static_cast<int32_t>(i + 1), system_descriptor.name.c_str(), system_instance.construction_duration / constructors_duration * 100.0);
        } else {
            printf("[ORDER] %3d %.65s\n", static_cast<int32_t>(i + 1), system_descriptor.name.c_str());
        }
    }
    printf("[ORDER] Reordering took %.3f seconds.\n", std::chrono::duration<float>(after_sort - before_sort).count());
#endif
}

void World::propagate_system(const size_t system_type, const size_t system_index) {
    assert(system_type < std::size(m_systems));

    std::vector<SystemManager::SystemDescriptor>& systems = SystemManager::m_systems[system_type];
    assert(system_index < systems.size());

    SystemManager::SystemDescriptor& system = systems[system_index];
    assert(system.tag_expression->test(m_active_tags));

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
            if (systems[preceding_system_index].tag_expression->test(m_active_tags)) {
                propagate_system(system_type, preceding_system_index);
            }
        }
    }

    m_system_order[system_type].push_back(system_index);

    propagate_state[system_index] = PropagateState::COMPLETED;
}

} // namespace hg
