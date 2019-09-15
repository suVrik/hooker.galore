#pragma once

#include <map>
#include <memory>
#include <string>

namespace hg {

/** `EditorMenuSingleComponent` is an interface to editor menu. You can create menu items with it. */
struct EditorMenuSingleComponent final {
    struct MenuItem final {
        explicit MenuItem(const std::shared_ptr<bool>& selected, const std::string& shortcut = {}, bool enabled = true) noexcept
                : selected(selected)
                , shortcut(shortcut)
                , enabled(enabled) {
        }

        std::shared_ptr<bool> selected;
        std::string shortcut;
        bool enabled;
    };

    std::map<std::string, MenuItem> items;
};

} // namespace hg
