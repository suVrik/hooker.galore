#include "core/ecs/world.h"
#include "world/shared/normal_input_single_component.h"
#include "world/shared/window_single_component.h"
#include "world/shared/window_system.h"

#include <fmt/format.h>
#include <iostream>
#include <SDL2/SDL.h>

namespace hg {

WindowSystem::WindowSystem(World& world)
        : NormalSystem(world) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(fmt::format("Failed to initialize SDL!\nDetails: {}", SDL_GetError()));
    }

    auto& window_single_component = world.set<WindowSingleComponent>();
    window_single_component.window = SDL_CreateWindow(window_single_component.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_single_component.width, window_single_component.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window_single_component.window == nullptr) {
        SDL_Quit();
        throw std::runtime_error(fmt::format("Failed to initialize a window!\nDetails: {}", SDL_GetError()));
    }

    world.set<NormalInputSingleComponent>();
}

WindowSystem::~WindowSystem() {
    auto& window_single_component = world.ctx<WindowSingleComponent>();
    SDL_DestroyWindow(window_single_component.window);
    SDL_Quit();
}

void WindowSystem::update(float /*elapsed_time*/) {
    auto& window_single_component = world.ctx<WindowSingleComponent>();
    window_single_component.resized = false;

    const char* window_title = SDL_GetWindowTitle(window_single_component.window);
    if (std::strcmp(window_title, window_single_component.title.c_str()) != 0) {
        SDL_SetWindowTitle(window_single_component.window, window_single_component.title.c_str());
    }

    int window_width, window_height;
    SDL_GetWindowSize(window_single_component.window, &window_width, &window_height);
    if (window_width != window_single_component.width || window_height != window_single_component.height) {
        SDL_SetWindowSize(window_single_component.window, window_single_component.width, window_single_component.height);
    }

    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();

    std::copy(std::begin(normal_input_single_component.m_mouse_buttons), std::end(normal_input_single_component.m_mouse_buttons), std::begin(normal_input_single_component.m_previous_mouse_buttons));
    std::copy(std::begin(normal_input_single_component.m_keys), std::end(normal_input_single_component.m_keys), std::begin(normal_input_single_component.m_previous_keys));

    normal_input_single_component.m_mouse_delta_x = 0;
    normal_input_single_component.m_mouse_delta_y = 0;
    normal_input_single_component.m_mouse_wheel = 0;
    normal_input_single_component.m_text[0] = '\0';

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                world.unset<RunningWorldSingleComponent>();
                return;
            case SDL_WINDOWEVENT:
                handle_window_event(event);
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
                handle_mouse_event(event);
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            case SDL_TEXTINPUT:
                handle_key_event(event);
                break;
            default:
                break;
        }
    }

#if defined(__APPLE__)
    normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_CTRL)] =
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_LGUI)] ||
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_RGUI)];
#else
    normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_CTRL)] =
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_LCTRL)] ||
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_RCTRL)];
#endif

    normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_SHIFT)] =
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_LSHIFT)] ||
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_RSHIFT)];

    normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_ALT)] =
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_LALT)] ||
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_RALT)];

#if defined(__APPLE__)
    normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_GUI)] =
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_LCTRL)] ||
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_RCTRL)];
#else
    normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_GUI)] =
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_LGUI)] ||
            normal_input_single_component.m_keys[static_cast<size_t>(Control::KEY_RGUI)];
#endif
}

void WindowSystem::handle_window_event(SDL_Event& event) {
    switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
            auto &window_single_component = world.ctx<WindowSingleComponent>();
            window_single_component.width = static_cast<uint32_t>(event.window.data1);
            window_single_component.height = static_cast<uint32_t>(event.window.data2);
            window_single_component.resized = true;
            break;
        }
        case SDL_WINDOWEVENT_FOCUS_LOST: {
            auto &normal_input_single_component = world.ctx<NormalInputSingleComponent>();
            for (bool& key : normal_input_single_component.m_keys) {
                key = false;
            }
            for (bool& button : normal_input_single_component.m_mouse_buttons) {
                button = false;
            }
        }
        default:
            break;
    }
}

void WindowSystem::handle_mouse_event(SDL_Event& event) {
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();
    switch (event.type) {
        case SDL_MOUSEMOTION:
            normal_input_single_component.m_mouse_x = event.motion.x;
            normal_input_single_component.m_mouse_y = event.motion.y;
            normal_input_single_component.m_mouse_delta_x = event.motion.xrel;
            normal_input_single_component.m_mouse_delta_y = event.motion.yrel;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if (event.button.button > 0 && event.button.button - 1 < std::size(normal_input_single_component.m_mouse_buttons)) {
                normal_input_single_component.m_mouse_buttons[event.button.button - 1] = event.button.state == SDL_PRESSED;
            }
            break;
        case SDL_MOUSEWHEEL:
            normal_input_single_component.m_mouse_wheel = event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? event.wheel.y : -event.wheel.y;
            break;
        default:
            break;
    }
}

void WindowSystem::handle_key_event(SDL_Event& event) {
    auto& normal_input_single_component = world.ctx<NormalInputSingleComponent>();
    switch (event.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (event.key.keysym.scancode < std::size(normal_input_single_component.m_keys)) {
                normal_input_single_component.m_keys[event.key.keysym.scancode] = event.type == SDL_KEYDOWN;
            }
            break;
        case SDL_TEXTINPUT:
            std::copy(std::begin(event.text.text), std::end(event.text.text), std::begin(normal_input_single_component.m_text));
            break;
        default:
            break;
    }
}

} // namespace hg
