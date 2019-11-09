#pragma once

#include <map>
#include <memory>
#include <string>

namespace hg {

class EditorMenuSystem;

/** `EditorMenuSingleComponent` is an interface to editor menu. You can create menu items with it. */
class EditorMenuSingleComponent final {
public:
    /** Add menu item with the specified `name`. */
    void add_item(const std::string& name, const std::shared_ptr<bool>& selected, const std::string& shortcut = {}, bool enabled = true) noexcept;

private:
    struct MenuItem final {
        std::weak_ptr<bool> selected;
        std::string shortcut;
        bool enabled;
    };

    std::map<std::string, MenuItem> m_items;

    friend class EditorMenuSystem;
};

// TODO: Move to a separate source file.
inline void EditorMenuSingleComponent::add_item(const std::string& name, const std::shared_ptr<bool>& selected, const std::string& shortcut, bool enabled) noexcept {
    m_items[name] = MenuItem{ std::weak_ptr<bool>(selected), shortcut, enabled };
}

} // namespace hg
