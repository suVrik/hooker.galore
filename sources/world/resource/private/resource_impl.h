#pragma once

#include "world/resource/resource.h"

namespace hg {

inline Resource::LoadingState Resource::get_loading_state() const {
    return m_loading_state;
}

inline bool Resource::is_loaded() const {
    return m_loading_state == LoadingState::LOADING;
}

} // namespace hg
