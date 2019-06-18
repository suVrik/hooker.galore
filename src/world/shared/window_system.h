#pragma once

#include "core/ecs/system.h"

union SDL_Event;

namespace hg {

/** `WindowSystem` fetches window events. */
class WindowSystem final : public NormalSystem {
public:
    explicit WindowSystem(World& world);
    ~WindowSystem() override;
    void update(float elapsed_time) override;

private:
    void handle_window_event(SDL_Event& event);
    void handle_mouse_event(SDL_Event& event);
    void handle_key_event(SDL_Event& event);
};

} // namespace hg
