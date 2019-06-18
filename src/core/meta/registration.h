#pragma once

#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp>

#define REFLECTION_CAT_IMPL(a, b) a##b
#define REFLECTION_CAT(a, b) REFLECTION_CAT_IMPL(a, b)

#define REFLECTION_REGISTRATION                                                      \
static void reflection_auto_register_reflection_function_();                         \
namespace                                                                            \
{                                                                                    \
    struct reflection__auto__register__                                              \
    {                                                                                \
        reflection__auto__register__()                                               \
        {                                                                            \
            reflection_auto_register_reflection_function_();                         \
        }                                                                            \
    };                                                                               \
}                                                                                    \
static const reflection__auto__register__ REFLECTION_CAT(auto_register__, __LINE__); \
static void reflection_auto_register_reflection_function_()
