#pragma once

#include "core/meta/registration.h"

#ifdef SYSTEM_DESCRIPTOR
#undef SYSTEM_DESCRIPTOR
#endif

/**
    Usage sample:

    SYSTEM_DESCRIPTOR(
        SYSTEM(FooSystem),
        REQUIRE("foo", "boo"),
        EXCLUSIVE("bar"),
        BEFORE("Following0System", "Following1System", "Following2System"),
        AFTER("Preceding0System", "Preceding1System")
    )

    Every argument except SYSTEM is optional. If system requires no tags, it's added to every tag combination.
*/
#define SYSTEM_DESCRIPTOR(system, ...) SYSTEM_DESCRIPTOR_IMPL(system, ##__VA_ARGS__)

#ifdef SYSTEM_DESCRIPTOR_IMPL
#undef SYSTEM_DESCRIPTOR_IMPL
#endif

#define SYSTEM_DESCRIPTOR_IMPL(system, ...) REFLECTION_REGISTRATION { \
    entt::reflect<system>(#system, ##__VA_ARGS__); \
}

#ifdef SYSTEM
#undef SYSTEM
#endif

#ifdef REQUIRE
#undef REQUIRE
#endif 

#ifdef EXCLUSIVE
#undef EXCLUSIVE
#endif 

#ifdef BEFORE
#undef BEFORE
#endif 

#ifdef AFTER
#undef AFTER
#endif 

#define SYSTEM(system) system
#define REQUIRE(...)   std::make_pair("require"_hs, std::vector<const char*>{ __VA_ARGS__ })
#define EXCLUSIVE(...) std::make_pair("exclusive"_hs, std::vector<const char*>{ __VA_ARGS__ })
#define BEFORE(...)    std::make_pair("before"_hs, std::vector<const char*>{ __VA_ARGS__ })
#define AFTER(...)     std::make_pair("after"_hs, std::vector<const char*>{ __VA_ARGS__ })
