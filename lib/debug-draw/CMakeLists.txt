cmake_minimum_required(VERSION 3.10)
project(debug-draw)

set(CMAKE_CXX_STANDARD 11)

add_library(debug-draw STATIC "${CMAKE_CURRENT_SOURCE_DIR}/source/debug_draw.cpp")

target_include_directories(debug-draw PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(debug-draw PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../glm/include/")

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
