#pragma once

#include "core/ecs/system.h"

namespace hg {

struct SelectedEntitySingleComponent;
class NormalInputSingleComponent;

/** `EntitySelectionSystem` shows Entity list UI, allows to pick an entity and stores it in
    `SelectedEntitySingleComponent`. */
class EntitySelectionSystem final : public NormalSystem {
public:
    explicit EntitySelectionSystem(World& world) noexcept;
    void update(float elapsed_time) override;

private:
    void show_level_window(SelectedEntitySingleComponent& selected_entity_single_component, NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void perform_picking(SelectedEntitySingleComponent& selected_entity_single_component, NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void delete_selected(SelectedEntitySingleComponent& selected_entity_single_component, NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void select_all(SelectedEntitySingleComponent& selected_entity_single_component, NormalInputSingleComponent& normal_input_single_component) const noexcept;
    void clear_selection(SelectedEntitySingleComponent& selected_entity_single_component, NormalInputSingleComponent& normal_input_single_component) const noexcept;
};

} // namespace hg
