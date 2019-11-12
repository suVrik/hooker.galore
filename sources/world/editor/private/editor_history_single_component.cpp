#include "core/ecs/world.h"
#include "world/editor/editor_history_single_component.h"
#include "world/editor/editor_selection_single_component.h"
#include "world/render/outline_component.h"
#include "world/shared/name_component.h"
#include "world/shared/name_single_component.h"

#ifdef ENABLE_HISTORY_LOG
#define HISTORY_LOG(message, ...) printf("[HISTORY] " message, ##__VA_ARGS__); fflush(stdout)
#else
#define HISTORY_LOG(message, ...)
#endif

namespace hg {

namespace history_single_component_details {

void perform_undo_redo(World& world, 
                       EditorHistorySingleComponent::HistoryChange& undo, 
                       EditorHistorySingleComponent::HistoryChange& redo) {
    auto& editor_selection_single_component = world.ctx<EditorSelectionSingleComponent>();
    auto& name_single_component = world.ctx<NameSingleComponent>();

    editor_selection_single_component.clear_selection(world);

    HISTORY_LOG("Performing %d undo-redo actions.\n", int32_t(undo.actions.size()));

    for (auto it = undo.actions.rbegin(); it != undo.actions.rend(); ++it) {
        EditorHistorySingleComponent::HistoryAction& undo_action = *it;
        EditorHistorySingleComponent::HistoryAction& redo_action = redo.actions.emplace_back();
        switch (undo_action.action_type) {
            case EditorHistorySingleComponent::ActionType::CREATE_ENTITY: {
                HISTORY_LOG("  Delete entity with name \"%s\".\n", undo_action.entity_name.c_str());

                assert(undo_action.components.empty());
                assert(name_single_component.name_to_entity.count(undo_action.entity_name) == 1);

                const entt::entity entity = name_single_component.name_to_entity[undo_action.entity_name];
                assert(world.valid(entity));

                assert(world.has<NameComponent>(entity));
                // Do not reference, all components are moved down there.
                const NameComponent name_component = world.get<NameComponent>(entity);
                assert(name_component.name == undo_action.entity_name);

                redo_action.action_type = EditorHistorySingleComponent::ActionType::DELETE_ENTITY;
                redo_action.entity_name = undo_action.entity_name;

                world.each_editable_component(entity, [&](const entt::meta_handle component) {
                    HISTORY_LOG("    Remember component \"%s\".\n", world.get_component_name(component.type()));
                    redo_action.components.push_back(ComponentManager::move_or_copy(component));
                });

                name_single_component.name_to_entity.erase(name_component.name);

                editor_selection_single_component.remove_from_selection(world, entity);
                world.destroy(entity);
                break;
            }
            case EditorHistorySingleComponent::ActionType::DELETE_ENTITY: {
                HISTORY_LOG("  Create entity with name \"%s\".\n", undo_action.entity_name.c_str());

                assert(name_single_component.name_to_entity.count(undo_action.entity_name) == 0);

                const entt::entity entity = world.create();
                for (entt::meta_any& component : undo_action.components) {
                    HISTORY_LOG("    Assign component \"%s\" to it.\n", world.get_component_name(component.type()));
                    world.assign_move_or_copy(entity, component);
                }

                assert(world.has<NameComponent>(entity));
                auto& name_component = world.get<NameComponent>(entity);
                assert(name_component.name == undo_action.entity_name);

                name_single_component.name_to_entity[name_component.name] = entity;

                redo_action.action_type = EditorHistorySingleComponent::ActionType::CREATE_ENTITY;
                redo_action.entity_name = undo_action.entity_name;

                editor_selection_single_component.add_to_selection(world, entity);
                break;
            }
            case EditorHistorySingleComponent::ActionType::ASSIGN_COMPONENT: {
                assert(undo_action.components.size() == 1);
                assert(name_single_component.name_to_entity.count(undo_action.entity_name) == 1);

                const entt::entity entity = name_single_component.name_to_entity[undo_action.entity_name];
                assert(world.valid(entity));
                assert(world.has<NameComponent>(entity));
                assert(world.get<NameComponent>(entity).name == undo_action.entity_name);
                assert(world.has(entity, undo_action.components.front().type()));

                world.remove(entity, undo_action.components.front().type());

                HISTORY_LOG("  Remove component \"%s\" from entity with name \"%s\".\n", world.get_component_name(undo_action.components.front().type()), undo_action.entity_name.c_str());

                redo_action.action_type = EditorHistorySingleComponent::ActionType::REMOVE_COMPONENT;
                redo_action.entity_name = undo_action.entity_name;
                redo_action.components.push_back(std::move(undo_action.components.front()));

                editor_selection_single_component.add_to_selection(world, entity);
                break;
            }
            case EditorHistorySingleComponent::ActionType::REMOVE_COMPONENT: {
                assert(undo_action.components.size() == 1);
                assert(name_single_component.name_to_entity.count(undo_action.entity_name) == 1);

                const entt::entity entity = name_single_component.name_to_entity[undo_action.entity_name];
                assert(world.valid(entity));
                assert(world.has<NameComponent>(entity));
                assert(world.get<NameComponent>(entity).name == undo_action.entity_name);
                assert(!world.has(entity, undo_action.components.front().type()));

                world.assign_move_or_copy(entity, undo_action.components.front());

                HISTORY_LOG("  Assign component \"%s\" to entity with name \"%s\".\n", world.get_component_name(undo_action.components.front().type()), undo_action.entity_name.c_str());

                redo_action.action_type = EditorHistorySingleComponent::ActionType::ASSIGN_COMPONENT;
                redo_action.entity_name = undo_action.entity_name;
                redo_action.components.push_back(std::move(undo_action.components.front()));

                editor_selection_single_component.add_to_selection(world, entity);
                break;
            }
            case EditorHistorySingleComponent::ActionType::REPLACE_COMPONENT: {
                assert(undo_action.components.size() == 1);
                assert(name_single_component.name_to_entity.count(undo_action.entity_name) == 1);

                const entt::entity entity = name_single_component.name_to_entity[undo_action.entity_name];
                assert(world.valid(entity));
                assert(world.has<NameComponent>(entity));
                assert(world.get<NameComponent>(entity).name == undo_action.entity_name);

                entt::meta_any& component = undo_action.components.front();
                const entt::meta_type component_type = component.type();
                assert(world.has(entity, undo_action.components.front().type()));

                const entt::meta_handle component_handle = world.get(entity, component_type);
                entt::meta_any component_copy = ComponentManager::copy(component_handle);

                std::string new_component_name;
                std::string old_component_name = undo_action.entity_name;
                if (component_type == entt::resolve<NameComponent>()) {
                    new_component_name = component.fast_cast<NameComponent>().name;
                    if (old_component_name != new_component_name) {
                        auto& name_single_component = world.ctx<NameSingleComponent>();

                        assert(name_single_component.name_to_entity.count(old_component_name) == 1);
                        assert(name_single_component.name_to_entity[old_component_name] == entity);
                        name_single_component.name_to_entity.erase(old_component_name);

                        assert(!new_component_name.empty());
                        assert(name_single_component.name_to_entity.count(new_component_name) == 0);
                        name_single_component.name_to_entity.emplace(new_component_name, entity);
                    }
                } else {
                    new_component_name = old_component_name;
                }

                HISTORY_LOG("  Replace component from entity with name \"%s\".\n", world.get_component_name(component_type), new_component_name.c_str());

                world.replace_move_or_copy(entity, component);

                redo_action.action_type = EditorHistorySingleComponent::ActionType::REPLACE_COMPONENT;
                redo_action.entity_name = new_component_name;
                redo_action.components.push_back(std::move(component_copy));

                editor_selection_single_component.add_to_selection(world, entity);
                break;
            }
        }
    }
}

} // namespace history_single_component_details

entt::entity EditorHistorySingleComponent::HistoryChange::create_entity(World& world, const std::string& name_hint) {
    auto& name_single_component = world.ctx<NameSingleComponent>();

    const entt::entity result = world.create();

    auto& name_component = world.assign<NameComponent>(result);
    name_component.name = name_single_component.acquire_unique_name(result, name_hint);

    HISTORY_LOG("Create entity with name \"%s\".\n", name_component.name.c_str());
    HISTORY_LOG("  Assign component \"NameComponent\" to it.\n");

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::CREATE_ENTITY;

    return result;
}

void EditorHistorySingleComponent::HistoryChange::delete_entity(World& world, const entt::entity entity) {
    auto& name_single_component = world.ctx<NameSingleComponent>();

    assert(world.valid(entity));
    assert(world.has<NameComponent>(entity));

    auto& name_component = world.get<NameComponent>(entity);

    HISTORY_LOG("Delete entity with name \"%s\".\n", name_component.name.c_str());

    assert(name_single_component.name_to_entity.count(name_component.name) == 1);
    name_single_component.name_to_entity.erase(name_component.name);

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::DELETE_ENTITY;
    action.entity_name = name_component.name;

    world.each_editable_component(entity, [&](const entt::meta_handle component) {
        HISTORY_LOG("  Remember component \"%s\".\n", world.get_component_name(component.type()));
        action.components.push_back(ComponentManager::move_or_copy(component));
    });

    world.destroy(entity);
}

void EditorHistorySingleComponent::HistoryChange::assign_component(World& world, const entt::entity entity, const entt::meta_handle component) {
    assert(world.valid(entity));
    assert(world.has<NameComponent>(entity));
    assert(component);
    assert(!world.has(entity, component.type()));
    assert(ComponentManager::is_editable(component.type()));

    HISTORY_LOG("Assign component \"%s\" to entity with name \"%s\".\n", world.get_component_name(component.type()), world.get<NameComponent>(entity).name.c_str());

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::ASSIGN_COMPONENT;
    action.entity_name = world.get<NameComponent>(entity).name;
    action.components.push_back(ComponentManager::copy(component));
}

entt::meta_handle EditorHistorySingleComponent::HistoryChange::assign_component_copy(World& world, const entt::entity entity, const entt::meta_handle component) {
    assign_component(world, entity, component);
    return world.assign_copy(entity, component);
}

entt::meta_handle EditorHistorySingleComponent::HistoryChange::assign_component_move(World& world, const entt::entity entity, const entt::meta_handle component) {
    assign_component(world, entity, component);
    return world.assign_move(entity, component);
}

void EditorHistorySingleComponent::HistoryChange::replace_component(World& world, const entt::entity entity, entt::meta_handle component) {
    assert(world.valid(entity));
    assert(world.has<NameComponent>(entity));
    assert(component);
    assert(world.has(entity, component.type()));
    assert(ComponentManager::is_editable(component.type()));

    std::string new_component_name;
    std::string old_component_name = world.get<NameComponent>(entity).name;
    if (component.type() == entt::resolve<NameComponent>()) {
        new_component_name = component.data<NameComponent>()->name;
        if (old_component_name != new_component_name) {
            auto& name_single_component = world.ctx<NameSingleComponent>();

            assert(name_single_component.name_to_entity.count(old_component_name) == 1);
            assert(name_single_component.name_to_entity[old_component_name] == entity);
            name_single_component.name_to_entity.erase(old_component_name);

            if (new_component_name.empty()) {
                new_component_name = "undefined";
            }

            if (name_single_component.name_to_entity.count(new_component_name) == 1) {
                new_component_name = name_single_component.acquire_unique_name(entity, new_component_name);
            } else {
                name_single_component.name_to_entity.emplace(new_component_name, entity);
            }

            assert(name_single_component.name_to_entity.count(new_component_name) == 1);
            assert(name_single_component.name_to_entity[new_component_name] == entity);
            component.data<NameComponent>()->name = new_component_name;
        }
    } else {
        new_component_name = old_component_name;
    }

    HISTORY_LOG("Replace component \"%s\" from entity with name \"%s\".\n", world.get_component_name(component.type()), new_component_name.c_str());

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::REPLACE_COMPONENT;
    action.entity_name = new_component_name;
    action.components.push_back(ComponentManager::copy(world.get(entity, component.type())));
}

entt::meta_handle EditorHistorySingleComponent::HistoryChange::replace_component_copy(World& world, const entt::entity entity, const entt::meta_handle component) {
    assert(ComponentManager::is_move_assignable(component.type()));
    replace_component(world, entity, component);
    return world.replace_copy(entity, component);
}

entt::meta_handle EditorHistorySingleComponent::HistoryChange::replace_component_move(World& world, const entt::entity entity, const entt::meta_handle component) {
    assert(ComponentManager::is_copy_assignable(component.type()));
    replace_component(world, entity, component);
    return world.replace_move(entity, component);
}

entt::meta_handle EditorHistorySingleComponent::HistoryChange::replace_component_move_or_copy(World& world, const entt::entity entity, const entt::meta_handle component) {
    replace_component(world, entity, component);
    if (ComponentManager::is_move_assignable(component.type())) {
        world.replace_move(entity, component);
    }
    return world.replace_copy(entity, component);
}

void EditorHistorySingleComponent::HistoryChange::remove_component(World& world, const entt::entity entity, const entt::meta_type component_type) {
    assert(world.valid(entity));
    assert(world.has<NameComponent>(entity));
    assert(world.has(entity, component_type));
    assert(ComponentManager::is_editable(component_type));

    HISTORY_LOG("Remove component \"%s\" from entity with name \"%s\".\n", world.get_component_name(component_type), world.get<NameComponent>(entity).name.c_str());

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::REMOVE_COMPONENT;
    action.entity_name = world.get<NameComponent>(entity).name;

    ComponentManager::move_or_copy(world.get(entity, component_type));

    world.remove(entity, component_type);
}

EditorHistorySingleComponent::HistoryChange* EditorHistorySingleComponent::begin(World& world, const std::string& description) {
    if (!is_continuous) {
        undo_position = (undo_position + 1) % HISTORY_BUFFER_SIZE;

        EditorHistorySingleComponent::HistoryChange& change = undo[undo_position];
        change.actions.clear();
        change.description = description;

        redo.clear();

        is_level_changed = true;

        return &change;
    }
    return nullptr;
}

EditorHistorySingleComponent::HistoryChange* EditorHistorySingleComponent::begin_continuous(World& world, const std::string& description) {
    if (!is_continuous) {
        EditorHistorySingleComponent::HistoryChange* result = begin(world, description);
        is_continuous = true;
        return result;
    }
    return nullptr;
}

void EditorHistorySingleComponent::end_continuous() {
    assert(is_continuous);
    is_continuous = false;
    is_level_changed = true;
}

void EditorHistorySingleComponent::perform_undo(World& world) {
    if (!is_continuous && !undo[undo_position].actions.empty()) {
        EditorHistorySingleComponent::HistoryChange& change = redo.emplace_back();
        change.description = std::move(undo[undo_position].description);

        history_single_component_details::perform_undo_redo(world, undo[undo_position], change);

        undo[undo_position].actions.clear();
        undo_position = (undo_position + HISTORY_BUFFER_SIZE - 1) % HISTORY_BUFFER_SIZE;

        is_level_changed = true;
    }
}

void EditorHistorySingleComponent::perform_redo(World& world) {
    if (!is_continuous && !redo.empty()) {
        undo_position = (undo_position + 1) % HISTORY_BUFFER_SIZE;

        EditorHistorySingleComponent::HistoryChange& change = undo[undo_position];
        change.actions.clear();
        change.description = std::move(redo.back().description);

        history_single_component_details::perform_undo_redo(world, redo.back(), change);

        redo.pop_back();

        is_level_changed = true;
    }
}

} // namespace hg
