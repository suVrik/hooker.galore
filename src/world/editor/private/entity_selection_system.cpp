#include "core/ecs/world.h"
#include "world/editor/editor_component.h"
#include "world/editor/entity_selection_system.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/render/outline_component.h"

#include <imgui.h>

namespace hg {

EntitySelectionSystem::EntitySelectionSystem(World& world) noexcept
        : NormalSystem(world) {
    world.set<SelectedEntitySingleComponent>();
}

void EntitySelectionSystem::update(float /*elapsed_time*/) {
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();

    world.sort<EditorComponent>([](const EditorComponent& a, const EditorComponent& b) {
        return a.name < b.name;
    });

    ImGui::SetNextWindowDockID(ImGui::GetID("Main"), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Level")) {
        size_t idx = 0;
        world.view<EditorComponent>().each([&](entt::entity entity, EditorComponent &editor_component) {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf;
            if (selected_entity_single_component.selected_entity == entity) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            ImGui::TreeNodeEx(reinterpret_cast<void*>(++idx), flags, "%s", editor_component.name.c_str());
            if (ImGui::IsItemClicked()) {
                if (world.valid(selected_entity_single_component.selected_entity)) {
                    world.reset<OutlineComponent>(selected_entity_single_component.selected_entity);
                }
                selected_entity_single_component.selected_entity = entity;
                world.assign<OutlineComponent>(entity);
            }
        });
    }
    ImGui::End();

    // TODO: Select by clicking on entities in world view.
}

} // namespace hg

