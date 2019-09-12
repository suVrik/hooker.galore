#pragma once

#include "core/ecs/system.h"

namespace hg {

struct GuidSingleComponent;
struct HistorySingleComponent;
struct NameSingleComponent;
struct SelectedEntitySingleComponent;

/** `PropertyEditorSystem` draws a grid in editor. */
class PropertyEditorSystem final : public NormalSystem {
public:
    explicit PropertyEditorSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    void edit_entities(SelectedEntitySingleComponent& selected_entity_single_component) const noexcept;
    void edit_component(GuidSingleComponent& guid_single_component, HistorySingleComponent& history_single_component,
                        NameSingleComponent& name_single_component, SelectedEntitySingleComponent& selected_entity_single_component,
                        const entt::meta_type component_type) const noexcept;
    void show_add_component_popup(HistorySingleComponent& history_single_component, SelectedEntitySingleComponent& selected_entity_single_component) const noexcept;
    void show_save_as_preset_popup() const noexcept;
    bool list_properties(SelectedEntitySingleComponent& selected_entity_single_component, const std::string& name, std::vector<entt::meta_any>& objects, bool is_component) const noexcept;
};

} // namespace hg
