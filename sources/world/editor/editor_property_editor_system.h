#pragma once

#include "core/ecs/system.h"

#include <entt/meta/meta.hpp>
#include <vector>

namespace hg {

struct EditorHistorySingleComponent;
struct EditorSelectionSingleComponent;
struct NameSingleComponent;

/** `EditorPropertyEditorSystem` draws a grid in editor. */
class EditorPropertyEditorSystem final : public NormalSystem {
public:
    explicit EditorPropertyEditorSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    void edit_entities(EditorSelectionSingleComponent& editor_selection_single_component) const noexcept;
    void edit_component(EditorHistorySingleComponent& editor_history_single_component,
                        NameSingleComponent& name_single_component,
                        EditorSelectionSingleComponent& editor_selection_single_component,
                        const entt::meta_type component_type) const noexcept;
    void show_add_component_popup(EditorHistorySingleComponent& editor_history_single_component,
                                  EditorSelectionSingleComponent& editor_selection_single_component) const noexcept;
    void show_save_as_preset_popup() const noexcept;
    bool list_properties(EditorSelectionSingleComponent& editor_selection_single_component,
                         const std::string& name, std::vector<entt::meta_any>& objects, bool is_component) const noexcept;
};

} // namespace hg
