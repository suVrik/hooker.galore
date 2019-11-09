#pragma once

#include "core/ecs/system.h"

namespace hg {

class NormalInputSingleComponent;
struct EditorFileSingleComponent;

/** `EditorFileSystem` manages "file" menu. */
class EditorFileSystem final : public NormalSystem {
public:
    explicit EditorFileSystem(World& world);
    void update(float elapsed_time) override;
    
private:
    void handle_new_level_action(EditorFileSingleComponent& editor_file_single_component, 
                                 NormalInputSingleComponent& normal_input_single_component);
    void handle_open_level_action(EditorFileSingleComponent& editor_file_single_component, 
                                  NormalInputSingleComponent& normal_input_single_component);
    void handle_save_level_action(EditorFileSingleComponent& editor_file_single_component, 
                                  NormalInputSingleComponent& normal_input_single_component);
    void handle_save_level_as_action(EditorFileSingleComponent& editor_file_single_component, 
                                     NormalInputSingleComponent& normal_input_single_component);
    void handle_save_level_popup(EditorFileSingleComponent& editor_file_single_component);
    void handle_save_level_as_popup(EditorFileSingleComponent& editor_file_single_component);
    void handle_open_level_popup(EditorFileSingleComponent& editor_file_single_component);
    void handle_level_error_popup(EditorFileSingleComponent& editor_file_single_component);
    void update_window_title();
    bool is_level_changed();
    bool new_level();
    void clear_level();
    bool save_level();
    bool open_level();
};

} // namespace hg
