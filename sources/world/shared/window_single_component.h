#pragma once

#include <cinttypes>
#include <string>

struct SDL_Window;

namespace hg {

/** `WindowSingleComponent` contains window-related data. */
struct WindowSingleComponent final {
    SDL_Window* window = nullptr;
    std::string title  = "Window";
    uint32_t width     = 800;
    uint32_t height    = 600;
    bool resized       = false;
};

} // namespace hg
