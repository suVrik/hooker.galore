#include "core/ecs/world.h"

namespace hg {

World::World() {
    set<RunningWorldSingleComponent>();
    register_components(*this);
    register_systems(*this);
}

World::~World() {
    for (int32_t i = 1; i >= 0; i--) {
        for (auto it = m_system_order[i].rbegin(); it != m_system_order[i].rend(); ++it) {
            const std::string& system_name = *it;
            assert(m_systems[i].count(system_name) > 0);
            m_systems[i][system_name].system = nullptr;
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

void World::normal_system_order(const std::vector<std::string>& system_order) noexcept {
    m_system_order[0] = system_order;
}

void World::fixed_system_order(const std::vector<std::string>& system_order) noexcept {
    m_system_order[1] = system_order;
}

void World::construct_systems() {
    for (size_t i = 0; i < 2; i++) {
        for (const std::string &system_name : m_system_order[i]) {
            assert(m_systems[i].count(system_name) > 0);
            if (!m_systems[i][system_name].system) {
                m_systems[i][system_name].system = m_systems[i][system_name].construct(*this);
                assert(m_systems[i][system_name].system);
            }
        }
    }
}

bool World::update_normal(float elapsed_time) noexcept {
    for (const std::string& system_name : m_system_order[0]) {
        assert(m_systems[0].count(system_name) > 0);
        assert(m_systems[0][system_name].system);
        if (try_ctx<RunningWorldSingleComponent>() != nullptr) {
            m_systems[0][system_name].system->update(elapsed_time);
        } else {
            return false;
        }
    }
    return true;
}

void World::update_fixed(float elapsed_time) noexcept {
    for (const std::string& system_name : m_system_order[1]) {
        assert(m_systems[1].count(system_name) > 0);
        assert(m_systems[1][system_name].system);

        m_systems[1][system_name].system->update(elapsed_time);
    }
}

} // namespace hg
