#include "core/ecs/world.h"
#include "world/editor/editor_component.h"
#include "world/editor/guid_single_component.h"
#include "world/editor/property_editor_system.h"
#include "world/editor/selected_entity_single_component.h"
#include "world/shared/name_single_component.h"

#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace hg {

PropertyEditorSystem::PropertyEditorSystem(World& world) noexcept
        : NormalSystem(world) {
}

void PropertyEditorSystem::update(float /*elapsed_time*/) {
    auto& guid_single_component = world.ctx<GuidSingleComponent>();
    auto& name_single_component = world.ctx<NameSingleComponent>();
    auto& selected_entity_single_component = world.ctx<SelectedEntitySingleComponent>();

    if (ImGui::Begin("Property Editor")) {
        if (selected_entity_single_component.selected_entities.size() == 1) {
            entt::entity entity = selected_entity_single_component.selected_entities[0];
            assert(world.valid(entity));

            uint32_t index = 0;
            world.each(entity, [&](entt::meta_handle component_handle) {
                entt::meta_type component_type = component_handle.type();
                if (component_type) {
                    std::string component_name;

                    entt::meta_prop component_name_property = component_type.prop("name"_hs);
                    if (component_name_property && component_name_property.value().can_cast<const char*>()) {
                        component_name = component_name_property.value().cast<const char*>();
                    } else {
                        component_name = "Undefined-" + std::to_string(++index);
                    }

                    if (ImGui::TreeNode(component_name.c_str())) {
                        uint32_t old_guid = 0;
                        std::string old_name;
                        if (component_type == entt::resolve<EditorComponent>()) {
                            auto* editor_component = component_handle.try_cast<EditorComponent>();
                            assert(editor_component != nullptr);

                            old_guid = editor_component->guid;
                            old_name = editor_component->name;
                        }

                        if (list_properties(component_handle)) {
                            if (component_type == entt::resolve<EditorComponent>()) {
                                auto* editor_component = component_handle.try_cast<EditorComponent>();
                                assert(editor_component != nullptr);

                                if (editor_component->guid != old_guid) {
                                    guid_single_component.guid_to_entity.erase(old_guid);
                                    if (guid_single_component.guid_to_entity.count(editor_component->guid & 0x00FFFFFF) > 0) {
                                        editor_component->guid = guid_single_component.acquire_unique_guid(entity);
                                    }
                                }

                                if (editor_component->name != old_name) {
                                    name_single_component.name_to_entity.erase(old_name);
                                    if (editor_component->name.empty()) {
                                        editor_component->name = "undefined";
                                    }
                                    if (name_single_component.name_to_entity.count(editor_component->name) > 0) {
                                        editor_component->name = name_single_component.acquire_unique_name(entity, editor_component->name);
                                    }
                                }
                            }

                            // Notify other systems about updated component.
                            world.replace(entity, component_handle);

                            // TODO: HISTORY changed component.
                        }
                        ImGui::TreePop();
                    }
                }
            });
        } else {
            if (!selected_entity_single_component.selected_entities.empty()) {
                // TODO: Edit properties of many entities at the same time.
                ImGui::TextWrapped("Editing properties of multiple entities is not yet supported.");
            } else {
                ImGui::TextUnformatted("No entity selected.");
            }
        }
    }
    ImGui::End();
}

bool PropertyEditorSystem::list_properties(entt::meta_handle object) const noexcept {
    bool is_changed = false;

    uint32_t idx = 0;
    entt::meta_type object_type = object.type();
    object_type.data([&](entt::meta_data data) {
        std::string data_name;

        entt::meta_prop data_name_property = data.prop("name"_hs);
        if (data_name_property && data_name_property.value().can_cast<const char*>()) {
            data_name = data_name_property.value().cast<const char*>();
        } else {
            data_name = "undefined-" + std::to_string(++idx);
        }

        entt::meta_type data_type = data.type();
        if (data_type.is_class()) {
            static entt::meta_type TYPE_VEC2   = entt::resolve<glm::vec2>();
            static entt::meta_type TYPE_VEC3   = entt::resolve<glm::vec3>();
            static entt::meta_type TYPE_VEC4   = entt::resolve<glm::vec4>();
            static entt::meta_type TYPE_QUAT   = entt::resolve<glm::quat>();
            static entt::meta_type TYPE_STRING = entt::resolve<std::string>();

            entt::meta_any data_copy = data.get(object);
            if (data_type == TYPE_VEC2) {
                glm::vec2 value = data_copy.cast<glm::vec2>();
                if (ImGui::InputFloat2(data_name.c_str(), &value.x)) {
                    data.set(object, value);
                    is_changed = true;
                }
            } else if (data_type == TYPE_VEC3) {
                glm::vec3 value = data_copy.cast<glm::vec3>();
                if (ImGui::InputFloat3(data_name.c_str(), &value.x)) {
                    data.set(object, value);
                    is_changed = true;
                }
            } else if (data_type == TYPE_VEC4) {
                glm::vec4 value = data_copy.cast<glm::vec4>();
                if (ImGui::InputFloat4(data_name.c_str(), &value.x)) {
                    data.set(object, value);
                    is_changed = true;
                }
            } else if (data_type == TYPE_QUAT) {
                const glm::quat value = data_copy.cast<glm::quat>();
                glm::vec3 euler_angles = glm::degrees(glm::eulerAngles(value));
                if (ImGui::InputFloat3(data_name.c_str(), &euler_angles.x)) {
                    data.set(object, glm::normalize(glm::quat(glm::radians(euler_angles))));
                    is_changed = true;
                }
            } else if (data_type == TYPE_STRING) {
                static char buffer[1024];
                const std::string string = data.get(object).cast<std::string>();
                std::strcpy(buffer, string.data());
                if (ImGui::InputText(data_name.c_str(), buffer, std::size(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    data.set(object, std::string(buffer));
                    is_changed = true;
                }
            } else {
                if (ImGui::TreeNode(data_name.c_str())) {
                    entt::meta_handle data_copy_handle = data_copy;
                    if (list_properties(data_copy_handle)) {
                        data.set(object, data_copy);
                        is_changed = true;
                    }
                    ImGui::TreePop();
                }
            }
        } else {
            static entt::meta_type TYPE_INT    = entt::resolve<int32_t>();
            static entt::meta_type TYPE_UINT   = entt::resolve<uint32_t>();
            static entt::meta_type TYPE_FLOAT  = entt::resolve<float>();
            static entt::meta_type TYPE_BOOL   = entt::resolve<bool>();

            if (data_type == TYPE_INT) {
                int32_t value = data.get(object).cast<int32_t>();
                if (ImGui::InputInt(data_name.c_str(), &value, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    data.set(object, value);
                    is_changed = true;
                }
            } else if (data_type == TYPE_UINT) {
                int32_t value = int32_t(data.get(object).cast<uint32_t>());
                if (ImGui::InputInt(data_name.c_str(), &value, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    data.set(object, static_cast<uint32_t>(value));
                    is_changed = true;
                }
            } else if (data_type == TYPE_FLOAT) {
                float value = data.get(object).cast<float>();
                if (ImGui::InputFloat(data_name.c_str(), &value, 0.f, 0.f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                    data.set(object, value);
                    is_changed = true;
                }
            } else if (data_type == TYPE_BOOL) {
                bool value = data.get(object).cast<bool>();
                if (ImGui::Checkbox(data_name.c_str(), &value)) {
                    data.set(object, value);
                    is_changed = true;
                }
            } else {
                ImGui::Text("Unsupported property \"%s\".", data_name.c_str());
            }
        }
    });

    return is_changed;
}

} // namespace hg
