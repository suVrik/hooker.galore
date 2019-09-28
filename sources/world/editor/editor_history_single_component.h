#pragma once

#include <array>
#include <entt/entity/registry.hpp>
#include <entt/meta/meta.hpp>
#include <string>
#include <variant>
#include <vector>

namespace hg {

class World;

/** `EditorHistorySingleComponent` contains history of changes on the level. */
struct EditorHistorySingleComponent final {
    static constexpr size_t HISTORY_BUFFER_SIZE = 512;

    /** All possible action types performed in editor. */
    enum class ActionType : uint32_t {
        CREATE_ENTITY,
        DELETE_ENTITY,
        ASSIGN_COMPONENT,
        REMOVE_COMPONENT,
        REPLACE_COMPONENT
    };

    /** `HistoryAction` contains data needed to perform an action of `ActionType`. */
    struct HistoryAction final {
        ActionType action_type;
        std::string entity_name;
        std::vector<entt::meta_any> components;
    };

    /** `HistoryChange` is a bunch of actions performed with one user action (e.g. translate multiple objects). */
    class HistoryChange final {
    public:
        /** Create entity and remember it in history. */
        entt::entity create_entity(World& world, const std::string& name_hint) noexcept;

        /** Delete entity and remember it in history. */
        void delete_entity(World& world, entt::entity entity) noexcept;

        /** Attach copy of specified component to given entity and remember it in history. */
        entt::meta_handle assign_component_copy(World& world, entt::entity entity, entt::meta_handle component) noexcept;
        entt::meta_handle assign_component_move(World& world, entt::entity entity, entt::meta_handle component) noexcept;

        /** Replace specified component in given entity and remember it in history. */
        entt::meta_handle replace_component_copy(World& world, entt::entity entity, entt::meta_handle component) noexcept;
        entt::meta_handle replace_component_move(World& world, entt::entity entity, entt::meta_handle component) noexcept;
        entt::meta_handle replace_component_move_or_copy(World& world, entt::entity entity, entt::meta_handle component) noexcept;

        /** Remove specified component from given entity and remember it in history. */
        void remove_component(World& world, entt::entity entity, entt::meta_type component_type) noexcept;

        std::vector<HistoryAction> actions;
        std::string description;

    private:
        void assign_component(World& world, entt::entity entity, entt::meta_handle component) noexcept;
        void replace_component(World& world, entt::entity entity, entt::meta_handle component) noexcept;
    };

    /** Clear `redo` stack, append new item with the specified `description` to `undo` buffer. */
    HistoryChange* begin(World& world, const std::string& description) noexcept;

    /** Clear `redo` stack, append new item with the specified `description` to `undo` buffer. Block undo buffer until
        `end_continuous` call. */
    HistoryChange* begin_continuous(World& world, const std::string& description) noexcept;
    void end_continuous() noexcept;

    /** Undo/redo recent changes. */
    void perform_undo(World& world) noexcept;
    void perform_redo(World& world) noexcept;

    std::array<HistoryChange, HISTORY_BUFFER_SIZE> undo;
    size_t undo_position = HISTORY_BUFFER_SIZE - 1;

    std::vector<HistoryChange> redo;

    bool is_continuous = false;
    bool is_level_changed = false;

    // Menu items.
    std::shared_ptr<bool> undo_action;
    std::shared_ptr<bool> redo_action;
};

} // namespace hg
