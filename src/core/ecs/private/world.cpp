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

entt::meta_handle World::assign(entt::entity entity, entt::meta_type component) noexcept {
    assert(m_components.count(component) != 0 && "Specified component type is not registered!");
    return m_components.find(component)->second.assign(this, entity);
}

entt::meta_handle World::assign(entt::entity entity, const entt::meta_any& component) noexcept {
    assert(m_components.count(component.type()) != 0 && "Specified component type is not registered!");
    return m_components.find(component.type())->second.assign_copy(this, entity, component.data());
}

entt::meta_handle World::assign(entt::entity entity, const entt::meta_handle& component) noexcept {
    assert(m_components.count(component.type()) != 0 && "Specified component type is not registered!");
    return m_components.find(component.type())->second.assign_copy(this, entity, component.data());
}

entt::meta_handle World::replace(entt::entity entity, const entt::meta_handle& component) noexcept {
    assert(m_components.count(component.type()) != 0 && "Specified component type is not registered!");
    return m_components.find(component.type())->second.replace(this, entity, component.data());
}

void World::remove(entt::entity entity, entt::meta_type component) noexcept {
    assert(m_components.count(component) != 0 && "Specified component type is not registered!");
    m_components.find(component)->second.remove(this, entity);
}

bool World::has(entt::entity entity, entt::meta_type component) const noexcept {
    assert(m_components.count(component) != 0 && "Specified component type is not registered!");
    return m_components.find(component)->second.has(this, entity);
}

entt::meta_handle World::get(entt::entity entity, entt::meta_type component) const noexcept {
    assert(m_components.count(component) != 0 && "Specified component type is not registered!");
    return m_components.find(component)->second.get(this, entity);
}

entt::meta_handle World::get_or_assign(entt::entity entity, entt::meta_type component) noexcept {
    assert(m_components.count(component) != 0 && "Specified component type is not registered!");
    return m_components.find(component)->second.get_or_assign(this, entity);
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
                m_current_system = system_name;
                m_systems[i][system_name].system = m_systems[0][system_name].construct(*this);
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
            m_current_system = system_name;
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

        m_current_system = system_name;
        m_systems[1][system_name].system->update(elapsed_time);
    }
}

bool World::before(const std::string& system_name) const noexcept {
    assert(m_current_system != system_name);
    auto normal_it = std::find(m_system_order[0].begin(), m_system_order[0].end(), m_current_system);
    if (normal_it != m_system_order[0].end()) {
        auto another_it = std::find(m_system_order[0].begin(), m_system_order[0].end(), system_name);
        return normal_it < another_it;
    } else {
        auto fixed_it = std::find(m_system_order[1].begin(), m_system_order[0].end(), m_current_system);
        assert(fixed_it != m_system_order[1].end());

        auto another_it = std::find(m_system_order[0].begin(), m_system_order[0].end(), system_name);
        return fixed_it < another_it;
    }
}

bool World::after(const std::string& system_name) const noexcept {
    assert(m_current_system != system_name);
    auto normal_it = std::find(m_system_order[0].begin(), m_system_order[0].end(), m_current_system);
    if (normal_it != m_system_order[0].end()) {
        auto another_it = std::find(m_system_order[0].begin(), m_system_order[0].end(), system_name);
        if (another_it != m_system_order[0].end()) {
            return normal_it > another_it;
        }
        return true;
    } else {
        auto fixed_it = std::find(m_system_order[1].begin(), m_system_order[0].end(), m_current_system);
        assert(fixed_it != m_system_order[1].end());

        auto another_it = std::find(m_system_order[0].begin(), m_system_order[0].end(), system_name);
        if (another_it != m_system_order[1].end()) {
            return fixed_it > another_it;
        }
        return true;
    }
}

} // namespace hg
