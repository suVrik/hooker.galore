#pragma once

#include "core/ecs/system.h"

namespace hg {

struct EditorFileSingleComponent;
class NormalInputSingleComponent;

/** `EditorFileSystem` manages "file" menu. */
class EditorFileSystem final : public NormalSystem {
public:
    explicit EditorFileSystem(World& world) noexcept;
    void update(float elapsed_time) override;
    
private:
    void handle_new_level_action(EditorFileSingleComponent& editor_file_single_component, NormalInputSingleComponent& normal_input_single_component) noexcept;
    void handle_open_level_action(EditorFileSingleComponent& editor_file_single_component, NormalInputSingleComponent& normal_input_single_component) noexcept;
    void handle_save_level_action(EditorFileSingleComponent& editor_file_single_component, NormalInputSingleComponent& normal_input_single_component) noexcept;
    void handle_save_level_as_action(EditorFileSingleComponent& editor_file_single_component, NormalInputSingleComponent& normal_input_single_component) noexcept;
    void handle_save_level_popup(EditorFileSingleComponent& editor_file_single_component) noexcept;
    void handle_save_level_as_popup(EditorFileSingleComponent& editor_file_single_component) noexcept;
    void handle_open_level_popup(EditorFileSingleComponent& editor_file_single_component) noexcept;
    void handle_level_error_popup(EditorFileSingleComponent& editor_file_single_component) noexcept;
    void update_window_title() noexcept;
    bool is_level_changed() noexcept;
    bool new_level() noexcept;
    void clear_level() noexcept;
    bool save_level() noexcept;
    bool open_level() noexcept;
};

} // namespace hg
