cmake_minimum_required(VERSION 3.10)
project(imgui)

set(CMAKE_CXX_STANDARD 11)

add_library(imgui STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/source/imgui.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/imgui_demo.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/imgui_draw.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/imgui_widgets.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/ImGuizmo.cpp"
)

target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_INCLUDE_IMCONFIG_H)


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
