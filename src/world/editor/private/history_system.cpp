#include "core/ecs/world.h"
#include "world/editor/history_single_component.h"
#include "world/editor/history_system.h"
#include "world/shared/normal_input_single_component.h"

#include <imgui.h>

namespace hg {

HistorySystem::HistorySystem(World& world) noexcept
        : NormalSystem(world) {
    world.set<HistorySingleComponent>();
}

void HistorySystem::update(float /*elapsed_time*/) {
    auto& history_single_component = world.ctx<HistorySingleComponent>();
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();

    if (ImGui::Begin("History")) {
        int32_t rewind_offset = 0;
        for (size_t i = 1; i <= HistorySingleComponent::HISTORY_BUFFER_SIZE; i++) {
            const size_t index = (history_single_component.undo_position + i) % HistorySingleComponent::HISTORY_BUFFER_SIZE;
            if (!history_single_component.undo[index].actions.empty()) {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx(reinterpret_cast<void*>(index), flags, "%s", history_single_component.undo[index].description.c_str());
                if (ImGui::IsItemClicked()) {
                    rewind_offset = -int32_t(HistorySingleComponent::HISTORY_BUFFER_SIZE - i);
                }
            }
        }
        for (auto it = history_single_component.redo.rbegin(); it != history_single_component.redo.rend(); ++it) {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf;
            ImGui::TreeNodeEx(reinterpret_cast<void*>(HistorySingleComponent::HISTORY_BUFFER_SIZE + std::distance(history_single_component.redo.rbegin(), it)), flags, "%s", it->description.c_str());
            if (ImGui::IsItemClicked()) {
                rewind_offset = 1 + int32_t(std::distance(history_single_component.redo.rbegin(), it));
            }
        }
        while (rewind_offset < 0) {
            history_single_component.perform_undo(world);
            rewind_offset++;
        }
        while (rewind_offset > 0) {
            history_single_component.perform_redo(world);
            rewind_offset--;
        }
    }
    ImGui::End();

    if ((normal_input_single_component.is_down(Control::KEY_LCTRL) || normal_input_single_component.is_down(Control::KEY_RCTRL)) &&
        (!normal_input_single_component.is_down(Control::KEY_LSHIFT) && !normal_input_single_component.is_down(Control::KEY_RSHIFT)) &&
        normal_input_single_component.is_pressed(Control::KEY_Z)) {
        history_single_component.perform_undo(world);
    }

    if ((normal_input_single_component.is_down(Control::KEY_LCTRL) || normal_input_single_component.is_down(Control::KEY_RCTRL)) &&
        (normal_input_single_component.is_down(Control::KEY_LSHIFT) || normal_input_single_component.is_down(Control::KEY_RSHIFT)) &&
        normal_input_single_component.is_pressed(Control::KEY_Z)) {
        history_single_component.perform_redo(world);
    }
}

} // namespace hg
