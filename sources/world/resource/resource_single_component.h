#pragma once

#include "world/resource/resource.h"

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace hg {

/** TODO: Description. */
class ResourceSingleComponent {
public:
    /** Construct `ResourceSingleComponent` with empty resource pools and a few resource threads waiting for pending
        resources in background. */
    ResourceSingleComponent();
    ~ResourceSingleComponent();

    /** Resource path is a path to compiled resource relative to compiled resources directory. Absolute path is a path
        to requested resource you can use for C++ streams, C fopen and so on. The reason behind why it is a member
        function of a single component rather than some static function you could access from anywhere is because this
        function needs a lot of state such as path to resource directory currently used (because you can substitute it
        with another, even on runtime) and mapping from compiled resources to original resources (used when original
        resource directory is used rather than compiled). */
    std::string get_absolute_path(const std::string& resource_path) const;

    /** Acquire resource of specified `ResourceType` with given `args`. `ResourceType` must have `load` member function
        with `ResourceSingleComponent` reference specified as first argument and followed by `Args`. This function may
        and often will return not loaded yet resource so you must check its loading state before accessing its data. */
    template <typename ResourceType, typename... Args>
    std::shared_ptr<ResourceType> acquire(const Args&... args);

    /** Return arguments you specified before when acquired the given `resource`. This is surely not the fastest
        function because it iterates over all `ResourceType` resources, so don't use it for actual game stuff.
        It's meant for editor stuff, so I don't see any reasons to sacrifice any game performance to get that small
        editor performance boost. */
    template <typename... Args, typename ResourceType>
    std::tuple<Args...> get_resource_arguments(const std::shared_ptr<ResourceType>& resource);

private:
    class PoolDescriptorBase {
    public:
        virtual ~PoolDescriptorBase() = default;
        virtual void dequeue(ResourceSingleComponent& resource_single_component) = 0;
    };

    template <typename ResourceType, typename... Args>
    class PoolDescriptor : public PoolDescriptorBase {
    public:
        template <typename T>
        using is_decay = std::is_same<T, std::decay_t<T>>;
        
        static_assert(std::is_base_of_v<Resource, ResourceType>);
        static_assert(std::is_decay<Args>::value && ...);

        static const size_t INDEX;

        void dequeue(ResourceSingleComponent& resource_single_component) override;

        std::map<std::tuple<Args...>, std::weak_ptr<ResourceType>> to_load;
        std::map<std::tuple<Args...>, std::weak_ptr<ResourceType>> loading;
        std::map<std::tuple<Args...>, std::weak_ptr<ResourceType>> loaded;
        std::mutex mutex;
    };

    static std::vector<std::unique_ptr<PoolDescriptorBase>(*)()> pool_constructors;

    std::vector<std::unique_ptr<PoolDescriptorBase>> m_pools;

    std::vector<std::thread> m_threads;
    std::atomic_bool m_threads_running;

    std::vector<PoolDescriptorBase*> m_pending_pools;
    std::mutex m_pending_pools_mutex;
    std::condition_variable m_pending_pools_condition_variable;
};

} // namespace hg

#include "world/resource/private/resource_single_component_impl.h"
