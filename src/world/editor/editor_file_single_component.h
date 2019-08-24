#pragma once

#include <memory>

namespace hg {

/** `EditorFileSingleComponent` contains everything related to "file" menu. */
struct EditorFileSingleComponent final {
    /** `SaveLevelAsAction` enum specifies what must be done after successful "Save Level As" dialog. */
    enum class SaveLevelAsAction {
        NONE,
        NEW_LEVEL
    };

    std::shared_ptr<bool> save_level_action;
    SaveLevelAsAction save_level_as_action = SaveLevelAsAction::NONE;
    std::array<char, 256> save_level_as_path;
    bool ignore_changed_level = false;
    std::string selected_level;

    // Menu items.
    std::shared_ptr<bool> new_level;
    std::shared_ptr<bool> open_level;
    std::shared_ptr<bool> save_level;
    std::shared_ptr<bool> save_level_as;
};

} // namespace hg
