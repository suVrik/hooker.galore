#include "world/shared/normal_input_single_component.h"

#include <cassert>
#include <iterator>

namespace hg {

bool NormalInputSingleComponent::is_down(Control control) const {
    if (static_cast<size_t>(control) < std::size(m_keys)) {
        return !m_disable_keyboard && m_keys[static_cast<size_t>(control)];
    }

    assert(static_cast<size_t>(control) < std::size(m_keys) + std::size(m_mouse_buttons));
    return !m_disable_mouse && m_mouse_buttons[static_cast<size_t>(control) - std::size(m_keys)];
}

bool NormalInputSingleComponent::is_pressed(Control control) const {
    if (static_cast<size_t>(control) < std::size(m_keys)) {
        return !m_disable_keyboard && m_keys[static_cast<size_t>(control)] && !(!m_previous_disable_keyboard && m_previous_keys[static_cast<size_t>(control)]);
    }

    assert(static_cast<size_t>(control) < std::size(m_keys) + std::size(m_mouse_buttons));
    return !m_disable_mouse && m_mouse_buttons[static_cast<size_t>(control) - std::size(m_keys)] && !(!m_previous_disable_mouse && m_previous_mouse_buttons[static_cast<size_t>(control) - std::size(m_keys)]);
}

bool NormalInputSingleComponent::is_released(Control control) const {
    if (static_cast<size_t>(control) < std::size(m_keys)) {
        return !(!m_disable_keyboard && m_keys[static_cast<size_t>(control)]) && !m_previous_disable_keyboard && m_previous_keys[static_cast<size_t>(control) - std::size(m_keys)];
    }

    assert(static_cast<size_t>(control) < std::size(m_keys) + std::size(m_mouse_buttons));
    return !(!m_disable_mouse && m_mouse_buttons[static_cast<size_t>(control) - std::size(m_keys)]) && !m_previous_disable_mouse && m_previous_mouse_buttons[static_cast<size_t>(control) - std::size(m_keys)];
}

int32_t NormalInputSingleComponent::get_mouse_x() const {
    return m_mouse_x;
}

int32_t NormalInputSingleComponent::get_mouse_y() const {
    return m_mouse_y;
}

int32_t NormalInputSingleComponent::get_delta_mouse_x() const {
    return m_mouse_delta_x;
}

int32_t NormalInputSingleComponent::get_delta_mouse_y() const {
    return m_mouse_delta_y;
}

int32_t NormalInputSingleComponent::get_mouse_wheel() const {
    return m_mouse_wheel;
}

const char* NormalInputSingleComponent::get_text() const {
    return m_text;
}

} // namespace hg
