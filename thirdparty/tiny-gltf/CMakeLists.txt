cmake_minimum_required(VERSION 3.10)
project(tiny-gltf)

set(CMAKE_CXX_STANDARD 11)

add_library(tiny-gltf STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/json.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/tiny_gltf.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/tiny_gltf.cpp"
)

target_include_directories(tiny-gltf PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

if(WIN32)
    set(cxx_compiler_flags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
    )
    foreach(cxx_compiler_flag ${cxx_compiler_flags})
        string(REPLACE "/MD" "/MT" ${cxx_compiler_flag} "${${cxx_compiler_flag}}")
    endforeach()
endif()
