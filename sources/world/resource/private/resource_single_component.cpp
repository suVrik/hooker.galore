#include "world/resource/resource_single_component.h"

namespace hg {

std::vector<std::unique_ptr<ResourceSingleComponent::PoolDescriptorBase>(*)()> ResourceSingleComponent::pool_constructors;

ResourceSingleComponent::ResourceSingleComponent()
        : m_threads_running(true) {
    size_t pools_count = pool_constructors.size();
    m_pools.reserve(pools_count);
    for (size_t i = 0; i < pools_count; i++) {
        m_pools.push_back(pool_constructors[i]());
        assert(m_pools.back() != nullptr);
    }

    constexpr size_t RESOURCE_THREADS_COUNT = 4;
    m_threads.reserve(RESOURCE_THREADS_COUNT);
    for (size_t i = 0; i < RESOURCE_THREADS_COUNT; i++) {
        m_threads.emplace_back([this] {
            while (true) {
                PoolDescriptorBase* pool_descriptor;

                {
                    std::unique_lock<std::mutex> lock(m_pending_pools_mutex);

                    if (m_pending_pools.empty() && m_threads_running) {
                        m_pending_pools_condition_variable.wait(lock);
                    }

                    if (!m_threads_running) {
                        return;
                    }
                    
                    assert(!m_pending_pools.empty());
                    pool_descriptor = m_pending_pools.back();
                    m_pending_pools.pop_back();
                }

                pool_descriptor->dequeue(*this);
            }
        });
    }
}


ResourceSingleComponent::~ResourceSingleComponent() {
    m_threads_running = false;
    m_pending_pools_condition_variable.notify_all();
    for (std::thread& thread : m_threads) {
        thread.join();
    }
}

std::string ResourceSingleComponent::get_absolute_path(const std::string& resource_path) const {
    // TODO
    return resource_path;
}

} // namespace hg
