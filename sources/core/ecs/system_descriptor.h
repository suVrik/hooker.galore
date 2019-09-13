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
        BEFORE("Following0System", "Following1System", "Following2System"),
        AFTER("Preceding0System", "Preceding1System")
    )

    Every argument except SYSTEM is optional. If system requires no tags, it's added to every tag combination.
*/
#define SYSTEM_DESCRIPTOR(system, ...) REFLECTION_REGISTRATION { \
    entt::reflect<system>(#system, ##__VA_ARGS__); \
}

#ifdef SYSTEM
#undef SYSTEM
#endif

#ifdef REQUIRE
#undef REQUIRE
#endif 

#ifdef BEFORE
#undef BEFORE
#endif 

#ifdef AFTER
#undef AFTER
#endif 

#define SYSTEM(system)            system
#define REQUIRE(required_tags)    std::make_pair("require", std::vector<const char*>{ required_tags })
#define BEFORE(following_systems) std::make_pair("before", std::vector<const char*>{ following_systems })
#define AFTER(preceding_system)   std::make_pair("after", std::vector<const char*>{ preceding_system })
