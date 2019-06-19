#include "core/ecs/world.h"
#include "world/shared/window_single_component.h"
#include "world/shared/level_single_component.h"

#include <chrono>
#include <clara.hpp>
#include <fmt/format.h>
#include <iostream>
#include <SDL2/SDL_messagebox.h>

int main(int argc, char* argv[]) {
    SDL_Window* message_box_window = nullptr;

    try {
        bool is_editor         = false;
        std::string level_file = "default.yaml";

        auto cli = clara::Opt(is_editor)["--editor"]("Run level editor") |
                   clara::Opt(level_file, "default.yaml")["--level"]("Level file to play/edit");
        if (auto result = cli.parse(clara::Args(argc, argv)); !result) {
            const std::string error_description = fmt::format("Error in command line: {}", result.errorMessage());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", error_description.c_str(), nullptr);
            return 1;
        }

        std::vector<std::string> normal_system_order = {
                "WindowSystem",
                "ImguiFetchSystem",
                "EditorCameraSystem",
                "CameraSystem",
                "RenderFetchSystem",
                "PresetSystem",
                "EntitySelectionSystem",
                "GizmoSystem",
                "ResourceSystem",
                "QuadSystem",
                "GeometryPassSystem",
                "LightingPassSystem",
                "DebugDrawPassSystem",
                "EditorGridSystem",
                "ImguiPassSystem",
                "RenderSystem",
        };
        std::vector<std::string> fixed_system_order;

        hg::World world;
        world.normal_system_order(normal_system_order);
        world.fixed_system_order(fixed_system_order);

        // `hg::LevelSingleComponent` is created and initialized before any system is executed.
        auto& level_single_component = world.set<hg::LevelSingleComponent>();
        level_single_component.level_name = level_file;

        world.construct_systems();

        // `hg::WindowSingleComponent` is created and initialized by `WindowSystem`.
        auto& window_single_component = world.ctx<hg::WindowSingleComponent>();
        if (is_editor) {
            window_single_component.title = "Hooker Galore Editor";
        } else {
            window_single_component.title = "Hooker Galore";
        }
        message_box_window = window_single_component.window;

        std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
        while (true) {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            if (!world.update_normal(std::chrono::duration<float>(now - last).count())) {
                break;
            }
            last = now;
        }
    }
    catch (const std::runtime_error& error) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", error.what(), message_box_window);
        std::cerr << "Runtime Error: " << error.what() << std::endl;
        return 1;
    }
    catch (...) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime Error", "Anonymous runtime error.", message_box_window);
        std::cerr << "Runtime Error: Anonymous runtime error." << std::endl;
        return 1;
    }

    return 0;
}
