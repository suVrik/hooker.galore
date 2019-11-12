#pragma once

#include "core/ecs/tags.h"
#include "core/meta/registration.h"

#include <memory>
#include <vector>

#ifdef SYSTEM_DESCRIPTOR
#undef SYSTEM_DESCRIPTOR
#endif

/**
    Usage sample:

    SYSTEM_DESCRIPTOR(
        SYSTEM(FooSystem),
        TAGS(foo && boo && !bar),
        BEFORE("Following0System", "Following1System", "Following2System"),
        AFTER("Preceding0System", "Preceding1System")
    )

    Every argument except SYSTEM is optional. If system doesn't require any tags, it's added to every tag combination.
*/
#define SYSTEM_DESCRIPTOR(system, ...) SYSTEM_DESCRIPTOR_IMPL(system, ##__VA_ARGS__)

#ifdef SYSTEM_DESCRIPTOR_IMPL
#undef SYSTEM_DESCRIPTOR_IMPL
#endif

#define SYSTEM_DESCRIPTOR_IMPL(system, ...) REFLECTION_REGISTRATION { \
    using namespace tags; \
    entt::reflect<system>(#system, ##__VA_ARGS__); \
}

#ifdef SYSTEM
#undef SYSTEM
#endif

#ifdef TAGS
#undef TAGS
#endif

#ifdef BEFORE
#undef BEFORE
#endif 

#ifdef AFTER
#undef AFTER
#endif 

#define SYSTEM(system) system
#define TAGS(tags)     std::make_pair("tags"_hs, std::shared_ptr<hg::TagWrapper>(new hg::TagWrapperTemplate(tags)))
#define BEFORE(...)    std::make_pair("before"_hs, std::vector<const char*>{ __VA_ARGS__ })
#define AFTER(...)     std::make_pair("after"_hs, std::vector<const char*>{ __VA_ARGS__ })
