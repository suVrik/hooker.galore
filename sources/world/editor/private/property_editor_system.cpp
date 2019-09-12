#include "core/ecs/world.h"
#include "world/editor/editor_component.h"
#include "world/editor/guid_single_component.h"
#include "world/editor/history_single_component.h"
#include "world/editor/property_editor_system.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/name_single_component.h"

#include <fmt/format.h>
#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <imgui.h>

namespace hg {

PropertyEditorSystem::PropertyEditorSystem(World& world) noexcept
        : NormalSystem(world) {
}

void PropertyEditorSystem::update(float /*elapsed_time*/) {
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();

    if (ImGui::Begin("Property Editor", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
        if (!selected_entity_single_component.selected_entities.empty()) {
            edit_entities(selected_entity_single_component);
        } else {
            ImGui::TextUnformatted("No entities selected.");
        }
    }
    ImGui::End();
}

void PropertyEditorSystem::edit_entities(SelectedEntitySingleComponent& selected_entity_single_component) const noexcept {
    auto& guid_single_component = world.ctx<GuidSingleComponent>();
    auto& history_single_component = world.ctx<HistorySingleComponent>();
    auto& name_single_component = world.ctx<NameSingleComponent>();

    const ImVec2 window_size = ImGui::GetWindowSize();
    if (ImGui::BeginChildFrame(ImGui::GetID("property_editor_child"), ImVec2(0.f, window_size.y - 60.f), ImGuiWindowFlags_NoBackground)) {
        // Notify user if there's no common components.
        bool any_common_component = false;

        world.each_editable_entity_component(selected_entity_single_component.selected_entities.front(), [&](const entt::meta_handle component) {
            for (const entt::entity entity : selected_entity_single_component.selected_entities) {
                if (!world.has(entity, component.type())) {
                    // All selected entities must have this component.
                    return;
                }
            }

            if (component.type() == entt::resolve<EditorComponent>() && selected_entity_single_component.selected_entities.size() > 1) {
                // Don't allow to edit `EditorComponent` component of multiple entities.
                return;
            }

            edit_component(guid_single_component, history_single_component, name_single_component, selected_entity_single_component, component.type());

            any_common_component = true;
        });

        if (!any_common_component) {
            // A single entity always has at least EditorComponent.
            ImGui::TextUnformatted("No common components.");
        }
    }
    ImGui::EndChildFrame();

    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("Add Component?");
    }
    ImGui::SameLine();
    if (ImGui::Button("Save As Preset")) {
        ImGui::OpenPopup("Save As Preset?");
    }

    show_add_component_popup(history_single_component, selected_entity_single_component);
    show_save_as_preset_popup();
}

void PropertyEditorSystem::edit_component(GuidSingleComponent& guid_single_component, HistorySingleComponent& history_single_component,
                                          NameSingleComponent& name_single_component, SelectedEntitySingleComponent& selected_entity_single_component,
                                          const entt::meta_type component_type) const noexcept {
    const char* const component_name = world.get_component_name(component_type);
    assert(component_name != nullptr);

    const bool is_tree_node_open = ImGui::TreeNode(component_name);

    // Allow to remove any component except `EditorComponent`.
    bool is_component_removed = false;
    if (component_type != entt::resolve<EditorComponent>()) {
        if (ImGui::BeginPopupContextItem(component_name)) {
            const std::string title(fmt::format("Remove component \"{}\"", component_name));
            if (ImGui::Selectable(title.c_str())) {
                auto* const change = history_single_component.begin(world, title);
                if (change != nullptr) {
                    for (const entt::entity entity : selected_entity_single_component.selected_entities) {
                        assert(world.has(entity, component_type));
                        change->remove_component(world, entity, component_type);
                    }
                    is_component_removed = true;
                }
            }
            ImGui::EndPopup();
        }
    }

    if (is_tree_node_open) {
        if (!is_component_removed) {
            std::vector<entt::meta_any> component_copies;
            component_copies.reserve(selected_entity_single_component.selected_entities.size());
            for (const entt::entity entity : selected_entity_single_component.selected_entities) {
                const entt::meta_handle component = world.get(entity, component_type);
                assert(component);

                entt::meta_any component_copy = world.copy_component(component);
                assert(component_copy);

                component_copies.push_back(std::move(component_copy));
            }

            if (list_properties(selected_entity_single_component, component_name, component_copies, true)) {
                auto* const change = history_single_component.begin(world, fmt::format("Change component \"{}\"", component_name));
                if (change != nullptr) {
                    // If `EditorComponent` was edited, update `GuidSingleComponent` and `NameSingleComponent`.
                    if (component_type == entt::resolve<EditorComponent>()) {
                        const entt::entity entity = selected_entity_single_component.selected_entities.front();

                        assert(selected_entity_single_component.selected_entities.size() == 1);
                        assert(component_copies.size() == 1);
                        assert(world.has<EditorComponent>(entity));

                        auto& old_editor_component = world.get<EditorComponent>(entity);
                        auto& new_editor_component = component_copies.front().fast_cast<EditorComponent>();

                        if (new_editor_component.guid != old_editor_component.guid) {
                            guid_single_component.guid_to_entity.erase(old_editor_component.guid);
                            if (guid_single_component.guid_to_entity.count(new_editor_component.guid & 0x00FFFFFF) > 0) {
                                new_editor_component.guid = guid_single_component.acquire_unique_guid(entity);
                            }
                        }

                        if (new_editor_component.name != old_editor_component.name) {
                            name_single_component.name_to_entity.erase(old_editor_component.name);
                            if (new_editor_component.name.empty()) {
                                new_editor_component.name = "undefined";
                            }
                            if (name_single_component.name_to_entity.count(new_editor_component.name) > 0) {
                                new_editor_component.name = name_single_component.acquire_unique_name(entity, new_editor_component.name);
                            }
                        }
                    }

                    // Assign new values.
                    for (size_t i = 0; i < component_copies.size(); i++) {
                        if (world.is_move_assignable(component_type)) {
                            change->replace_component_move(world, selected_entity_single_component.selected_entities[i], component_copies[i]);
                        } else {
                            change->replace_component_copy(world, selected_entity_single_component.selected_entities[i], component_copies[i]);
                        }
                    }
                }
            }
        }
        ImGui::TreePop();
    }
}

void PropertyEditorSystem::show_add_component_popup(HistorySingleComponent& history_single_component, SelectedEntitySingleComponent& selected_entity_single_component) const noexcept {
    if (ImGui::BeginPopupModal("Add Component?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto any_has = [&](const entt::meta_type component_type) {
            for (const entt::entity entity : selected_entity_single_component.selected_entities) {
                if (world.has(entity, component_type)) {
                    return true;
                }
            }
            return false;
        };

        if (!selected_entity_single_component.selected_component_to_add || any_has(selected_entity_single_component.selected_component_to_add)) {
            selected_entity_single_component.selected_component_to_add = entt::meta_type();
            world.each_editable_component_type([&](const entt::meta_type component_type) {
                if (!selected_entity_single_component.selected_component_to_add && !any_has(component_type)) {
                    selected_entity_single_component.selected_component_to_add = component_type;
                }
            });
        }

        if (selected_entity_single_component.selected_component_to_add) {
            assert(world.is_component_editable(selected_entity_single_component.selected_component_to_add));
            assert(!any_has(selected_entity_single_component.selected_component_to_add));

            if (ImGui::BeginCombo("Components", world.get_component_name(selected_entity_single_component.selected_component_to_add))) {
                world.each_editable_component_type([&](const entt::meta_type component_type) {
                    if (!any_has(component_type)) {
                        const bool is_selected = component_type == selected_entity_single_component.selected_component_to_add;
                        if (ImGui::Selectable(world.get_component_name(component_type), is_selected)) {
                            selected_entity_single_component.selected_component_to_add = component_type;
                        }
                    }
                });
                ImGui::EndCombo();
            }
        }

        ImGui::Spacing();
        ImGui::Indent(125.f);

        if (ImGui::Button("Cancel")) {
            selected_entity_single_component.selected_component_to_add = entt::meta_type();
            ImGui::CloseCurrentPopup();
        }

        if (selected_entity_single_component.selected_component_to_add) {
            assert(world.is_component_editable(selected_entity_single_component.selected_component_to_add));
            assert(!any_has(selected_entity_single_component.selected_component_to_add));

            ImGui::SameLine();
            if (ImGui::Button("Add Component")) {
                const char* const component_name = world.get_component_name(selected_entity_single_component.selected_component_to_add);
                auto* const change = history_single_component.begin(world, fmt::format("Add component \"{}\"", component_name));
                if (change != nullptr) {
                    for (const entt::entity entity : selected_entity_single_component.selected_entities) {
                        assert(!world.has(entity, selected_entity_single_component.selected_component_to_add));

                        entt::meta_any component = world.construct_component(selected_entity_single_component.selected_component_to_add);
                        change->assign_component_move(world, entity, component);
                    }
                }

                selected_entity_single_component.selected_component_to_add = entt::meta_type();
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}

void PropertyEditorSystem::show_save_as_preset_popup() const noexcept {
    if (ImGui::BeginPopupModal("Save As Preset?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        // TODO: Implement preset saving.

        static char TEMP[256] = {};
        if (ImGui::InputText("Name", TEMP, sizeof(TEMP))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::Spacing();
        ImGui::Indent(100.f);

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Save Preset")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

bool PropertyEditorSystem::list_properties(SelectedEntitySingleComponent& selected_entity_single_component, const std::string& name, std::vector<entt::meta_any>& objects, const bool is_component) const noexcept {
    assert(!objects.empty());

    const entt::meta_type object_type = objects.front().type();

#ifndef NDEBUG
    // All objects must have exactly the same type.
    for (size_t i = 1; i < objects.size(); i++) {
        assert(objects[i].type() == object_type);
    }
#endif

    const entt::meta_func editor_function = object_type.func("editor"_hs);
    const entt::meta_func compare_function = object_type.func("compare"_hs);
    if (editor_function && compare_function) {
        // Validate compare function signature.
        assert(compare_function.is_static());
        assert(compare_function.size() == 2);
        assert(compare_function.arg(0) == entt::resolve<entt::meta_handle>());
        assert(compare_function.arg(1) == entt::resolve<entt::meta_handle>());
        assert(compare_function.ret() == entt::resolve<bool>());

        const entt::meta_handle test_object = objects.front();

#ifndef NDEBUG
        // Comparing an object with itself must always return true.
        entt::meta_any test_compare_result = compare_function.invoke(entt::meta_handle(), test_object, test_object);
        assert(test_compare_result);
        assert(test_compare_result.type() == entt::resolve<bool>());
        assert(test_compare_result.fast_cast<bool>());
#endif

        // Check whether all selected entities have this property equal.
        bool is_equal = true;
        for (size_t i = 1; is_equal && i < objects.size(); i++) {
            entt::meta_any compare_result = compare_function.invoke(entt::meta_handle(), test_object, entt::meta_handle(objects[i]));
            assert(compare_result);
            assert(compare_result.type() == entt::resolve<bool>());
            if (!compare_result.fast_cast<bool>()) {
                is_equal = false;
            }
        }
        
        // If all selected entities have different property values, highlight that.
        if (!is_equal) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
        }

        // Validate editor function signature.
        assert(editor_function.is_static());
        assert(editor_function.size() == 2);
        assert(editor_function.arg(0) == entt::resolve<const char*>());
        assert(editor_function.arg(1) == entt::resolve<entt::meta_handle>());
        assert(editor_function.ret() == entt::resolve<bool>());

        entt::meta_any result = editor_function.invoke(entt::meta_handle(), name.c_str(), test_object);
        assert(result);
        assert(result.type() == entt::resolve<bool>());

        if (!is_equal) {
            ImGui::PopStyleColor(1);
        }

        if (result.fast_cast<bool>()) {
            for (size_t i = 1; i < objects.size(); i++) {
                objects[i] = objects[0];

                // Check copy constructor to be used.
                assert(objects[i].data() != objects[0].data());
            }
            return true;
        }
        return false;
    } else if (object_type.is_class()) {
        const entt::meta_func advanced_editor_function = object_type.func("advanced_editor"_hs);
        if (advanced_editor_function) {
            assert(editor_function.is_static());
            assert(editor_function.size() == 3);
            assert(editor_function.arg(0) == entt::resolve<const char*>());
            assert(editor_function.arg(1) == entt::resolve<const entt::meta_any*>());
            assert(editor_function.arg(2) == entt::resolve<size_t>());

            editor_function.invoke(entt::meta_handle(), name.c_str(), objects.data(), objects.size());
        } else {
            bool object_changed = false;
            if (is_component || ImGui::TreeNode(name.c_str())) {
                object_type.data([&](const entt::meta_data data) {
                    std::string data_name("undefined");

                    const entt::meta_prop data_name_property = data.prop("name"_hs);
                    if (data_name_property && data_name_property.value().type() == entt::resolve<const char*>()) {
                        data_name = data_name_property.value().fast_cast<const char*>();
                    }

                    std::vector<entt::meta_any> data_objects;
                    data_objects.reserve(objects.size());
                    for (entt::meta_any& object : objects) {
                        // Push copies of original properties to `data_objects`.
                        data_objects.push_back(data.get(object));
                    }

                    if (list_properties(selected_entity_single_component, data_name, data_objects, false)) {
                        for (size_t i = 0; i < objects.size(); i++) {
                            data.set(objects[i], data_objects[i]);
                        }
                        object_changed = true;
                    }
                });
                if (!is_component) {
                    ImGui::TreePop();
                }
            }
            return object_changed;
        }
    } else {
        ImGui::Text("Unsupported property \"%s\".", name.c_str());
        return false;
    }
}

} // namespace hg
