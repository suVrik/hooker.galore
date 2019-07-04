cmake_minimum_required(VERSION 3.10)
project(hooker_galore)

include(cmake/shaderc.cmake)
include(cmake/texturec.cmake)
include(cmake/deploy.cmake)

# Compile small thirdpary libraries in place.

add_subdirectory("${CMAKE_SOURCE_DIR}/lib/imgui/")
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/debug-draw/")
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/tiny-gltf/")

# Deploy the executable into binary directory.

set(CMAKE_CXX_STANDARD 17)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT hooker_galore)

# Compile executable sources.

file(GLOB_RECURSE HOOKER_GALORE_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(hooker_galore ${HOOKER_GALORE_SOURCES})

# Include thirdparty libraries.

target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/src/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/SDL2/${CMAKE_HOST_SYSTEM_NAME}/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/imgui/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/Clara/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/yaml-cpp/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/debug-draw/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/entt/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/glm/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/libfmt/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/tiny-gltf/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/ghc-filesystem/include/")

# Link thirdpary libraries.

target_link_libraries(hooker_galore PRIVATE imgui)
target_link_libraries(hooker_galore PRIVATE debug-draw)
target_link_libraries(hooker_galore PRIVATE tiny-gltf)

if(WIN32)
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/SDL2/Windows/lib/SDL2main.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/SDL2/Windows/lib/SDL2.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/bgfx.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/bimg.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/bx.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/astc-codec.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/yaml-cpp/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/yaml-cpp.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/libfmt/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/fmt.lib")
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(METAL_LIBRARY Metal)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    target_link_libraries(hooker_galore PRIVATE "${COCOA_LIBRARY}" "${METAL_LIBRARY}" "${QUARTZCORE_LIBRARY}")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/SDL2/Darwin/bin/libSDL2.dylib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libbgfx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libbimg.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libbx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libastc-codec.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/yaml-cpp/lib/Darwin/libyaml-cpp.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/libfmt/lib/Darwin/libfmt.a")
elseif(UNIX)
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)
    target_link_libraries(hooker_galore PRIVATE ${OPENGL_LIBRARIES})
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/SDL2/Linux/lib/libSDL2main.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/SDL2/Linux/lib/libSDL2.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Linux/libbgfx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Linux/libbimg.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Linux/libbx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Linux/libastc-codec.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/yaml-cpp/lib/Linux/libyaml-cpp.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/libfmt/lib/Linux/libfmt.a")
    target_link_libraries(hooker_galore PRIVATE ${CMAKE_DL_LIBS})
    target_link_libraries(hooker_galore PRIVATE pthread)
    target_link_libraries(hooker_galore PRIVATE X11)
endif()

# Link against static runtime library.

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

# Set up shared libraries location directory for MacOS and Linux.

if(APPLE)
    set_property(TARGET hooker_galore PROPERTY SKIP_BUILD_RPATH "ON")

    if(APPLE)
        set_property(TARGET hooker_galore APPEND PROPERTY LINK_FLAGS "-Wl,-rpath,\"@executable_path\"")
    else()
        set_property(TARGET hooker_galore APPEND PROPERTY LINK_FLAGS "-Wl,-rpath,\"$ORIGIN\"")
    endif()
endif()

# Compile shaders.

build_shaders("fragment" "${CMAKE_SOURCE_DIR}/src/shaders/*.fragment.sc" "${CMAKE_SOURCE_DIR}/src/shaders/headers/")
build_shaders("vertex" "${CMAKE_SOURCE_DIR}/src/shaders/*.vertex.sc" "${CMAKE_SOURCE_DIR}/src/shaders/headers/")
add_dependencies(hooker_galore build_fragment_shaders build_vertex_shaders)

# Compile textures.

set(output_directory "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/")
set(output_resources_directory "${output_directory}/resources/")
set(output_textures_directory "${output_resources_directory}/textures/")

build_textures("base_color_roughness" "${CMAKE_SOURCE_DIR}/res/textures/" "*_bcr.png" "${output_textures_directory}")
build_textures("normal_metal_ambient_occlusion" "${CMAKE_SOURCE_DIR}/res/textures/" "*_nmao.png" "${output_textures_directory}")
build_textures("height" "${CMAKE_SOURCE_DIR}/res/textures/" "*_h.png" "${output_textures_directory}")
build_textures("cube" "${CMAKE_SOURCE_DIR}/res/textures/" "*.hdr" "${output_textures_directory}")
add_dependencies(hooker_galore
        build_base_color_roughness_maps
        build_normal_metal_ambient_occlusion_maps
        build_cube_maps
        build_height_maps
)

# Deploy other resources.

deploy_resources("level" "${CMAKE_SOURCE_DIR}/res/levels/" "*.yaml" "${output_resources_directory}/levels/")
deploy_resources("model" "${CMAKE_SOURCE_DIR}/res/models/" "*.glb" "${output_resources_directory}/models/")
deploy_resources("preset" "${CMAKE_SOURCE_DIR}/res/presets/" "*.yaml" "${output_resources_directory}/presets/")
deploy_resources("texture" "${CMAKE_SOURCE_DIR}/res/textures/" "*.dds" "${output_resources_directory}/textures/")
deploy_resources("material" "${CMAKE_SOURCE_DIR}/res/materials/" "*.yaml" "${output_resources_directory}/materials/")
add_dependencies(hooker_galore
        deploy_levels
        deploy_models
        deploy_presets
        deploy_materials
)

# Deploy shared libraries.

deploy_shared_library("sdl2"
        "${output_directory}"
        "${CMAKE_SOURCE_DIR}/lib/SDL2/Windows/bin/SDL2.dll"
        "${CMAKE_SOURCE_DIR}/lib/SDL2/Darwin/bin/libSDL2.dylib"
        "${CMAKE_SOURCE_DIR}/lib/SDL2/Linux/bin/libSDL2.so"
)
add_dependencies(hooker_galore deploy_sdl2)
