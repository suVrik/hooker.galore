#include "core/ecs/world.h"
#include "world/editor/editor_tags.h"
#include "world/imgui/imgui_tags.h"
#include "world/physics/physics_tags.h"
#include "world/render/render_tags.h"
#include "world/shared/level_single_component.h"

#include <SDL2/SDL_messagebox.h>
#include <algorithm>
#include <chrono>
#include <clara.hpp>
#include <fmt/format.h>
#include <iostream>

namespace hg {

void register_systems();
void register_components();

} // namespace hg

int main(int argc, char* argv[]) {
    // Initialize `SystemManager` and `ComponentManager`.
    hg::register_systems();
    hg::register_components();

    try {
        // Move to a system constructor without any tag.
        // <ALL THIS> ///////////////

        bool is_editor         = false;
        std::string level_file = "default.yaml";

        auto cli = clara::Opt(is_editor)["--editor"]("Run level editor") |
                   clara::Opt(level_file, "default.yaml")["--level"]("Level file to play/edit");
        if (auto result = cli.parse(clara::Args(argc, argv)); !result) {
            const std::string error_description = fmt::format("Error in command line: {}", result.errorMessage());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", error_description.c_str(), nullptr);
            return 1;
        }

        hg::World world;
        world.add_tags(hg::tags::editor, hg::tags::render, hg::tags::imgui, hg::tags::physics);

        // `hg::LevelSingleComponent` is created and initialized before any system is executed.
        // TODO: This is temporary.
        auto& level_single_component = world.set<hg::LevelSingleComponent>();
        level_single_component.level_name = level_file;

        // </ALL THIS> ///////////////
        // Move to a system constructor without any tag (or tag like `entry`).

        std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
        while (true) {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            world.update_fixed(1.f / 60.f); // TODO: Call `update_fixed` from one of normal update systems.
            if (!world.update_normal(std::clamp(std::chrono::duration<float>(now - last).count(), 0.001f, 0.1f))) {
                break;
            }
            last = now;
        }
    }
    catch (const std::runtime_error& error) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", error.what(), nullptr);
        std::cerr << "Runtime Error: " << error.what() << std::endl;
        return 1;
    }
    catch (...) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", "Anonymous runtime error.", nullptr);
        std::cerr << "Runtime Error: Anonymous runtime error." << std::endl;
        return 1;
    }

    return 0;
}
