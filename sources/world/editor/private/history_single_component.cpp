#include "core/ecs/world.h"
#include "world/editor/editor_component.h"
#include "world/editor/guid_single_component.h"
#include "world/editor/history_single_component.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/name_single_component.h"
#include "world/shared/render/outline_component.h"

#ifdef ENABLE_HISTORY_LOG
#define HISTORY_LOG(message, ...) printf("[HISTORY] " message, ##__VA_ARGS__); fflush(stdout)
#else
#define HISTORY_LOG(message, ...)
#endif

namespace hg {

namespace history_single_component_details {

void perform_undo_redo(World& world, HistorySingleComponent::HistoryChange& undo, HistorySingleComponent::HistoryChange& redo) {
    auto& guid_single_component = world.ctx<GuidSingleComponent>();
    auto& name_single_component = world.ctx<NameSingleComponent>();
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();

    selected_entity_single_component.clear_selection(world);

    HISTORY_LOG("Performing %d undo-redo actions.\n", int32_t(undo.actions.size()));

    for (auto it = undo.actions.rbegin(); it != undo.actions.rend(); ++it) {
        HistorySingleComponent::HistoryAction& undo_action = *it;
        HistorySingleComponent::HistoryAction& redo_action = redo.actions.emplace_back();
        switch (undo_action.action_type) {
            case HistorySingleComponent::ActionType::CREATE_ENTITY: {
                HISTORY_LOG("  Delete entity with GUID %u.\n", undo_action.entity_guid);

                assert(undo_action.components.empty());
                assert(guid_single_component.guid_to_entity.count(undo_action.entity_guid) == 1);

                entt::entity entity = guid_single_component.guid_to_entity[undo_action.entity_guid];
                assert(world.valid(entity));

                assert(world.has<EditorComponent>(entity));
                auto& editor_component = world.get<EditorComponent>(entity);
                assert(editor_component.guid == undo_action.entity_guid);
                assert(name_single_component.name_to_entity.count(editor_component.name) == 1);

                redo_action.action_type = HistorySingleComponent::ActionType::DELETE_ENTITY;
                redo_action.entity_guid = undo_action.entity_guid;

                world.each_editable_entity_component(entity, [&](const entt::meta_handle component) {
                    HISTORY_LOG("    Remember component \"%s\".\n", world.get_component_name(component.type()));
                    if (world.is_move_constructible(component.type())) {
                        redo_action.components.push_back(world.move_component(component));
                    } else {
                        redo_action.components.push_back(world.copy_component(component));
                    }
                });

                guid_single_component.guid_to_entity.erase(editor_component.guid);
                name_single_component.name_to_entity.erase(editor_component.name);

                selected_entity_single_component.remove_from_selection(world, entity);
                world.destroy(entity);
                break;
            }
            case HistorySingleComponent::ActionType::DELETE_ENTITY: {
                HISTORY_LOG("  Create entity with GUID %u.\n", undo_action.entity_guid);

                assert(guid_single_component.guid_to_entity.count(undo_action.entity_guid) == 0);

                entt::entity entity = world.create();
                for (entt::meta_any& component : undo_action.components) {
                    HISTORY_LOG("    Assign component \"%s\" to it.\n", world.get_component_name(component.type()));
                    if (world.is_move_constructible(component.type())) {
                        world.assign_move(entity, component);
                    } else {
                        world.assign_copy(entity, component);
                    }
                }

                assert(world.has<EditorComponent>(entity));
                auto& editor_component = world.get<EditorComponent>(entity);
                assert(editor_component.guid == undo_action.entity_guid);
                assert(name_single_component.name_to_entity.count(editor_component.name) == 0);

                guid_single_component.guid_to_entity[editor_component.guid] = entity;
                name_single_component.name_to_entity[editor_component.name] = entity;

                redo_action.action_type = HistorySingleComponent::ActionType::CREATE_ENTITY;
                redo_action.entity_guid = editor_component.guid;

                selected_entity_single_component.add_to_selection(world, entity);
                break;
            }
            case HistorySingleComponent::ActionType::ASSIGN_COMPONENT: {
                assert(undo_action.components.size() == 1);
                assert(guid_single_component.guid_to_entity.count(undo_action.entity_guid) == 1);

                entt::entity entity = guid_single_component.guid_to_entity[undo_action.entity_guid];
                assert(world.valid(entity));
                assert(world.has<EditorComponent>(entity));
                assert(world.get<EditorComponent>(entity).guid == undo_action.entity_guid);
                assert(world.has(entity, undo_action.components.front().type()));

                world.remove(entity, undo_action.components.front().type());

                HISTORY_LOG("  Remove component \"%s\" from entity with GUID %u.\n", world.get_component_name(undo_action.components.front().type()), undo_action.entity_guid);

                redo_action.action_type = HistorySingleComponent::ActionType::REMOVE_COMPONENT;
                redo_action.entity_guid = undo_action.entity_guid;
                redo_action.components.push_back(std::move(undo_action.components.front()));

                selected_entity_single_component.add_to_selection(world, entity);
                break;
            }
            case HistorySingleComponent::ActionType::REMOVE_COMPONENT: {
                assert(undo_action.components.size() == 1);
                assert(guid_single_component.guid_to_entity.count(undo_action.entity_guid) == 1);

                entt::entity entity = guid_single_component.guid_to_entity[undo_action.entity_guid];
                assert(world.valid(entity));
                assert(world.has<EditorComponent>(entity));
                assert(world.get<EditorComponent>(entity).guid == undo_action.entity_guid);
                assert(!world.has(entity, undo_action.components.front().type()));

                entt::meta_any& component = undo_action.components.front();
                if (world.is_move_constructible(component.type())) {
                    world.assign_move(entity, component);
                } else {
                    world.assign_copy(entity, component);
                }

                HISTORY_LOG("  Assign component \"%s\" to entity with GUID %u.\n", world.get_component_name(undo_action.components.front().type()), undo_action.entity_guid);

                redo_action.action_type = HistorySingleComponent::ActionType::ASSIGN_COMPONENT;
                redo_action.entity_guid = undo_action.entity_guid;
                redo_action.components.push_back(std::move(undo_action.components.front()));

                selected_entity_single_component.add_to_selection(world, entity);
                break;
            }
            case HistorySingleComponent::ActionType::REPLACE_COMPONENT: {
                assert(undo_action.components.size() == 1);
                assert(guid_single_component.guid_to_entity.count(undo_action.entity_guid) == 1);

                const entt::entity entity = guid_single_component.guid_to_entity[undo_action.entity_guid];
                assert(world.valid(entity));
                assert(world.has<EditorComponent>(entity));
                assert(world.get<EditorComponent>(entity).guid == undo_action.entity_guid);
                assert(world.has(entity, undo_action.components.front().type()));

                const entt::meta_type component_type = undo_action.components.front().type();
                const entt::meta_handle component_handle = world.get(entity, component_type);
                entt::meta_any component_copy = world.copy_component(component_handle);

                HISTORY_LOG("  Replace component from entity with GUID %u.\n", world.get_component_name(component_type), undo_action.entity_guid);

                entt::meta_any& component = undo_action.components.front();
                if (world.is_move_assignable(component.type())) {
                    world.replace_move(entity, component);
                } else {
                    world.replace_copy(entity, component);
                }

                redo_action.action_type = HistorySingleComponent::ActionType::REPLACE_COMPONENT;
                redo_action.entity_guid = undo_action.entity_guid;
                redo_action.components.push_back(std::move(component_copy));

                selected_entity_single_component.add_to_selection(world, entity);
                break;
            }
        }
    }
}

} // namespace history_single_component_details

entt::entity HistorySingleComponent::HistoryChange::create_entity(World& world, const std::string& name_hint) noexcept {
    auto& guid_single_component = world.ctx<GuidSingleComponent>();
    auto& name_single_component = world.ctx<NameSingleComponent>();

    const entt::entity result = world.create();

    auto& editor_component = world.assign<EditorComponent>(result);
    editor_component.guid = guid_single_component.acquire_unique_guid(result);
    editor_component.name = name_single_component.acquire_unique_name(result, name_hint);

    HISTORY_LOG("Create entity with GUID %u.\n", editor_component.guid);
    HISTORY_LOG("  Assign component \"EditorComponent\" to it.\n");

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::CREATE_ENTITY;
    action.entity_guid = editor_component.guid;

    return result;
}

void HistorySingleComponent::HistoryChange::delete_entity(World& world, const entt::entity entity) noexcept {
    auto& guid_single_component = world.ctx<GuidSingleComponent>();
    auto& name_single_component = world.ctx<NameSingleComponent>();

    assert(world.valid(entity));
    assert(world.has<EditorComponent>(entity));

    auto& editor_component = world.get<EditorComponent>(entity);

    HISTORY_LOG("Delete entity with GUID %u.\n", editor_component.guid);

    assert(guid_single_component.guid_to_entity.count(editor_component.guid) == 1);
    guid_single_component.guid_to_entity.erase(editor_component.guid);

    assert(name_single_component.name_to_entity.count(editor_component.name) == 1);
    name_single_component.name_to_entity.erase(editor_component.name);

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::DELETE_ENTITY;
    action.entity_guid = editor_component.guid;

    world.each_editable_entity_component(entity, [&](const entt::meta_handle component) {
        HISTORY_LOG("  Remember component \"%s\".\n", world.get_component_name(component.type()));
        if (world.is_move_constructible(component.type())) {
            action.components.push_back(world.move_component(component));
        } else {
            action.components.push_back(world.copy_component(component));
        }
    });

    world.destroy(entity);
}

void HistorySingleComponent::HistoryChange::assign_component(World& world, const entt::entity entity, const entt::meta_handle component) noexcept {
    assert(world.valid(entity));
    assert(world.has<EditorComponent>(entity));
    assert(component);
    assert(!world.has(entity, component.type()));
    assert(world.is_component_editable(component.type()));

    HISTORY_LOG("Assign component \"%s\" to entity with GUID %u.\n", world.get_component_name(original.type()), world.get<EditorComponent>(entity).guid);

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::ASSIGN_COMPONENT;
    action.entity_guid = world.get<EditorComponent>(entity).guid;
    action.components.push_back(world.copy_component(component));
}

entt::meta_handle HistorySingleComponent::HistoryChange::assign_component_copy(World& world, const entt::entity entity, const entt::meta_handle component) noexcept {
    assign_component(world, entity, component);
    return world.assign_copy(entity, component);
}

entt::meta_handle HistorySingleComponent::HistoryChange::assign_component_move(World& world, const entt::entity entity, const entt::meta_handle component) noexcept {
    assign_component(world, entity, component);
    return world.assign_move(entity, component);
}

void HistorySingleComponent::HistoryChange::replace_component(World& world, const entt::entity entity, const entt::meta_handle component) noexcept {
    assert(world.valid(entity));
    assert(world.has<EditorComponent>(entity));
    assert(component);
    assert(world.has(entity, component.type()));
    assert(world.is_component_editable(component.type()));

    HISTORY_LOG("Replace component \"%s\" from entity with GUID %u.\n", world.get_component_name(original.type()), world.get<EditorComponent>(entity).guid);

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::REPLACE_COMPONENT;
    action.entity_guid = world.get<EditorComponent>(entity).guid;
    action.components.push_back(world.copy_component(world.get(entity, component.type())));
}

entt::meta_handle HistorySingleComponent::HistoryChange::replace_component_copy(World& world, const entt::entity entity, const entt::meta_handle component) noexcept {
    replace_component(world, entity, component);
    return world.replace_copy(entity, component);
}

entt::meta_handle HistorySingleComponent::HistoryChange::replace_component_move(World& world, const entt::entity entity, const entt::meta_handle component) noexcept {
    replace_component(world, entity, component);
    return world.replace_move(entity, component);
}

void HistorySingleComponent::HistoryChange::remove_component(World& world, const entt::entity entity, const entt::meta_type component_type) noexcept {
    assert(world.valid(entity));
    assert(world.has<EditorComponent>(entity));
    assert(world.has(entity, component_type));
    assert(world.is_component_editable(component_type));

    HISTORY_LOG("Remove component \"%s\" from entity with GUID %u.\n", world.get_component_name(original.type()), world.get<EditorComponent>(entity).guid);

    HistoryAction& action = actions.emplace_back();
    action.action_type = ActionType::REMOVE_COMPONENT;
    action.entity_guid = world.get<EditorComponent>(entity).guid;

    const entt::meta_handle component = world.get(entity, component_type);
    if (world.is_move_constructible(component_type)) {
        action.components.push_back(world.move_component(component));
    } else {
        action.components.push_back(world.copy_component(component));
    }

    world.remove(entity, component_type);
}

HistorySingleComponent::HistoryChange* HistorySingleComponent::begin(World& world, const std::string& description) noexcept {
    if (!is_continuous) {
        undo_position = (undo_position + 1) % HISTORY_BUFFER_SIZE;

        HistorySingleComponent::HistoryChange& change = undo[undo_position];
        change.actions.clear();
        change.description = description;

        redo.clear();

        is_level_changed = true;

        return &change;
    }
    return nullptr;
}

HistorySingleComponent::HistoryChange* HistorySingleComponent::begin_continuous(World& world, const std::string& description) noexcept {
    if (!is_continuous) {
        HistorySingleComponent::HistoryChange* result = begin(world, description);
        is_continuous = true;
        return result;
    }
    return nullptr;
}

void HistorySingleComponent::end_continuous() noexcept {
    assert(is_continuous);
    is_continuous = false;
    is_level_changed = true;
}

void HistorySingleComponent::perform_undo(World& world) noexcept {
    if (!is_continuous && !undo[undo_position].actions.empty()) {
        HistorySingleComponent::HistoryChange& change = redo.emplace_back();
        change.description = std::move(undo[undo_position].description);

        history_single_component_details::perform_undo_redo(world, undo[undo_position], change);

        undo[undo_position].actions.clear();
        undo_position = (undo_position + HISTORY_BUFFER_SIZE - 1) % HISTORY_BUFFER_SIZE;

        is_level_changed = true;
    }
}

void HistorySingleComponent::perform_redo(World& world) noexcept {
    if (!is_continuous && !redo.empty()) {
        undo_position = (undo_position + 1) % HISTORY_BUFFER_SIZE;

        HistorySingleComponent::HistoryChange& change = undo[undo_position];
        change.actions.clear();
        change.description = std::move(redo.back().description);

        history_single_component_details::perform_undo_redo(world, redo.back(), change);

        redo.pop_back();

        is_level_changed = true;
    }
}

} // namespace hg
