#pragma once

#include "world/resource/resource_single_component.h"

namespace hg {

namespace resource_single_component_details {

template <typename ResourceType, typename... Args, size_t... Indices>
bool load(ResourceType* value, ResourceSingleComponent& resource_single_component, const std::tuple<Args...>& key, std::index_sequence<Indices...>) {
    return value->load(resource_single_component, std::get<Indices>(key)...);
}

} // namespace resource_single_component_details

template <typename ResourceType, typename... Args>
std::shared_ptr<ResourceType> ResourceSingleComponent::acquire(const Args&... args) {
    // This `INDEX` usage instantiates a template which registers a resource pool before `main` execution.
    size_t pool_index = PoolDescriptor<ResourceType, std::decay_t<Args>...>::INDEX;
    assert(pool_index < m_pools.size());

    auto* pool_descriptor = static_cast<PoolDescriptor<ResourceType, std::decay_t<Args>...>*>(m_pools[pool_index].get());
    assert(pool_descriptor != nullptr);

    std::lock_guard<std::mutex> lock(pool_descriptor->mutex);

    if (auto loaded_it = pool_descriptor->loaded.find(key); loaded_it != pool_descriptor->loaded.end()) {
        std::shared_ptr<ResourceType> result(loaded_it->second.lock());
        if (result) {
            // Resource is loaded and has strong references.
            return result;
        } else {
            // Resource was loaded before, but lost all the strong references and died.
            pool_descriptor->loaded.erase(loaded_it);

            // Load it again.
            std::shared_ptr<ResourceType> result(new ResourceType());
            [[maybe_unused]] auto& [_, is_inserted] = pool_descriptor->to_load.emplace(key, result);
            assert(is_inserted); // `to_load` map shouldn't contain an item with the same key.
            m_pending_pools.push_back(pool_descriptor);
            return result;
        }
    } else {
        if (auto loading_it = pool_descriptor->loading.find(key); loading_it != pool_descriptor->loading.end()) {
            // Resource is loading, which means there must be at least one strong reference in a loading thread.
            std::shared_ptr<ResourceType> result(loading_it->second.lock());
            assert(result);
            return result;
        } else {
            if (auto to_load_it = pool_descriptor->to_load.find(key); to_load_it != pool_descriptor->to_load.end()) {
                std::shared_ptr<ResourceType> result(to_load_it->second.lock());
                if (result) {
                    // Resource is queued and has strong references.
                    return result;
                } else {
                    // Resource was queued before, but lost all the strong references and died. Revive it.
                    result = std::shared_ptr<ResourceType>(new ResourceType());
                    to_load_it->second = result;
                    return result;
                }
            } else {
                // Resource needs to be queued.
                std::shared_ptr<ResourceType> result(new ResourceType());
                pool_descriptor->to_load.emplace(key, result);
                m_pending_pools.push_back(pool_descriptor);
                return result;
            }
        }
    }
}

template <typename... Args, typename ResourceType>
std::tuple<Args...> ResourceSingleComponent::get_resource_arguments(const std::shared_ptr<ResourceType>& resource) {
    // This `INDEX` usage instantiates a template which registers a resource pool before `main` execution.
    size_t pool_index = PoolDescriptor<ResourceType, std::decay_t<Args>...>::INDEX;
    assert(pool_index < m_pools.size());

    auto* pool_descriptor = static_cast<PoolDescriptor<ResourceType, std::decay_t<Args>...>*>(m_pools[pool_index].get());
    assert(pool_descriptor != nullptr);

    std::lock_guard<std::mutex> lock(pool_descriptor->mutex);

    switch (resource->get_loading_state()) {
        case Resource::LoadingState::QUEUED: {
            auto it = pool_descriptor->to_load.find(key);
            assert(it != pool_descriptor->to_load.end());
            return it->first;
        }
        case Resource::LoadingState::LOADING: {
            auto it = pool_descriptor->loading.find(key);
            assert(it != pool_descriptor->loading.end());
            return it->first;
        }
        case Resource::LoadingState::LOADED:
        case Resource::LoadingState::ERROR: {
            auto it = pool_descriptor->loaded.find(key);
            assert(it != pool_descriptor->loaded.end());
            return it->first;
        }
    }
}

template <typename ResourceType, typename... Args>
const size_t ResourceSingleComponent::PoolDescriptor<ResourceType, Args...>::INDEX = ([] {
    size_t pool_index = ResourceSingleComponent::constructors.size();
    ResourceSingleComponent::constructors.push_back([]() {
        return std::unique_ptr<PoolDescriptorBase>(new PoolDescriptor<ResourceType, Args...>());
    });
    return pool_index;
})();

template <typename ResourceType, typename... Args>
void ResourceSingleComponent::PoolDescriptor<ResourceType, Args...>::dequeue(ResourceSingleComponent& resource_single_component) {
    std::tuple<Args...> key;
    std::shared_ptr<ResourceType> value;

    {
        std::lock_guard<std::mutex> lock(mutex);

        // `dequeue` shouldn't be called if to_load is empty.
        assert(!to_load.empty());
        auto it = to_load.begin();

        key = it->first;
        if (value = it->second.lock()) {
            // We can't steal key, because it's const, but we can steal value because we're about to remove it.
            [[maybe_unused]] auto& [_, is_inserted] = loading.emplace(key, std::move(it->second));
            assert(is_inserted); // `loading` map shouldn't contain an item with the same key.

            value->state = Resource::LoadingState::LOADING;
        } // Otherwise resource stored in a queue is expired already and doesn't have any strong references.

        to_load.erase(it);
    }

    if (value) {
        bool is_loaded = resource_single_component_details::load(value.get(), resource_single_component, key, std::index_sequence_for<Args...>());

        std::lock_guard<std::mutex> lock(mutex);
        
        [[maybe_unused]] size_t removed_count = loading.erase(key);
        assert(removed_count == 1); // `key` must be present in a loading queue.

        // Even if `is_loaded` is false, put it in `loaded` map so we don't have to load it again on another request.
        [[maybe_unused]] auto& [_, is_inserted] = loaded.emplace(std::move(key), value);
        assert(is_inserted); // `loaded` map shouldn't contain an item with the same key.

        value->state = is_loaded ? Resource::LoadingState::LOADED : Resource::LoadingState::ERROR;
    } // Otherwise resource stored in a queue is expired already and doesn't have any strong references.
}

} // namespace hg
