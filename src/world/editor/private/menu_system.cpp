#include "core/base/split.h"
#include "core/ecs/world.h"
#include "world/editor/menu_single_component.h"
#include "world/editor/menu_system.h"

#include <imgui.h>

namespace hg {

MenuSystem::MenuSystem(World& world) noexcept
    : NormalSystem(world) {
    world.set<MenuSingleComponent>();
}

void MenuSystem::update(float /*elapsed_time*/) {
    auto& menu_single_component = world.ctx<MenuSingleComponent>();
    if (ImGui::BeginMainMenuBar()) {
        std::vector<std::string> items;
        bool is_last_item_open = true;

        for (auto& [path, menu_item] : menu_single_component.items) {
            std::vector<std::string> current_items = split(path, '/');

            auto get_item_text = [](const std::string& full_text) {
                for (auto it = full_text.begin(); it != full_text.end(); ++it) {
                    if (!std::isdigit(*it)) {
                        return full_text.substr(std::distance(full_text.begin(), it));
                    }
                }
                return full_text;
            };

            auto items_it = items.begin();
            for (auto current_items_it = current_items.begin(); current_items_it != current_items.end(); ++current_items_it) {
                if (items_it != items.end()) {
                    if (*items_it != *current_items_it) {
                        for (auto it = items_it; it != items.end(); ++it) {
                            if (std::next(it) != items.end() || is_last_item_open) {
                                ImGui::EndMenu();
                            }
                        }
                        items.erase(items_it, items.end());
                        items_it = items.end();
                        is_last_item_open = true;
                    } else {
                        ++items_it;
                        continue;
                    }
                }
                if (is_last_item_open) {
                    if (std::next(current_items_it) != current_items.end()) {
                        items.push_back(*current_items_it);
                        items_it = items.end();

                        is_last_item_open = ImGui::BeginMenu(get_item_text(*current_items_it).c_str());
                        if (!is_last_item_open) {
                            break;
                        }
                    } else {
                        ImGui::MenuItem(get_item_text(*current_items_it).c_str(), menu_item.shortcut.c_str(), menu_item.selected.get(), menu_item.enabled);
                    }
                }
            }
        }
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (std::next(it) != items.end() || is_last_item_open) {
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();
    }
}

} // namespace hg
