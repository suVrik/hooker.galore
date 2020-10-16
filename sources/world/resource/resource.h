#pragma once

#include <atomic>

namespace hg {

/** `Resource` is an interface for all resources that can be loaded by `ResourceSystem`. It allows to query resource
    loading state. */
class Resource {
public:
    /** `LoadingState` defines resource's loading state. */
    enum class LoadingState {
        QUEUED,  ///< Resource is queued.
        LOADING, ///< Resource is loading.
        LOADED,  ///< Resource is loaded.
        ERROR,   ///< Error occurred during loading.
    };

    /** Return loading state of this resource. */
    LoadingState get_loading_state() const;

    /** Check whether this resource is loaded or not. */
    bool is_loaded() const;

private:
    std::atomic<LoadingState> m_loading_state{ LoadingState::QUEUED };
};

} // namespace hg

#include "world/resource/private/resource_impl.h"
