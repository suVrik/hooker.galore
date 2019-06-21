cmake_minimum_required(VERSION 3.10)
project(hooker_galore)

# Deploy the executable into binary directory.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# Compile Hooker Galore sources.

file(GLOB_RECURSE HOOKER_GALORE_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(hooker_galore ${HOOKER_GALORE_SOURCES})
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/src/")

# Include thirdparty libraries.

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
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/stb/include/")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/ghc-filesystem/include/")

# Link thirdpary libraries.

if(WIN32)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        # Link standard C and C++ libraries statically in Release. Link winpthread statically as well.
        target_link_libraries(hooker_galore PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive)
    endif()
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/SDL2/Windows/lib/libSDL2main.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/SDL2/Windows/lib/libSDL2.a" mingw32)
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Windows/${CMAKE_BUILD_TYPE}/libbgfx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Windows/${CMAKE_BUILD_TYPE}/libbimg.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Windows/${CMAKE_BUILD_TYPE}/libbx.a" psapi)
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Windows/${CMAKE_BUILD_TYPE}/libastc-codec.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/imgui/lib/Windows/${CMAKE_BUILD_TYPE}/libimgui.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/yaml-cpp/lib/Windows/${CMAKE_BUILD_TYPE}/libyaml-cpp.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/debug-draw/lib/Windows/${CMAKE_BUILD_TYPE}/libdebug_draw.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/libfmt/lib/Windows/${CMAKE_BUILD_TYPE}/libfmt.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/tiny-gltf/lib/Windows/${CMAKE_BUILD_TYPE}/libtiny_gltf.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/stb/lib/Windows/${CMAKE_BUILD_TYPE}/libstb_image.a")
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(METAL_LIBRARY Metal)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    target_link_libraries(hooker_galore PRIVATE "${COCOA_LIBRARY}" "${METAL_LIBRARY}" "${QUARTZCORE_LIBRARY}")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/SDL2/Darwin/lib/SDL2")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Darwin/libbgfx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Darwin/libbimg.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Darwin/libbx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/bgfx/lib/Darwin/libastc-codec.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/imgui/lib/Darwin/libimgui.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/yaml-cpp/lib/Darwin/libyaml-cpp.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/debug-draw/lib/Darwin/libdebug_draw.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/libfmt/lib/Darwin/libfmt.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/tiny-gltf/lib/Darwin/libtiny_gltf.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/stb/lib/Darwin/libstb_image.a")
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
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/imgui/lib/Linux/libimgui.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/yaml-cpp/lib/Linux/libyaml-cpp.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/debug-draw/lib/Linux/libdebug_draw.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/libfmt/lib/Linux/libfmt.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/tiny-gltf/lib/Linux/libtiny_gltf.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/lib/stb/lib/Linux/libstb_image.a")
    target_link_libraries(hooker_galore PRIVATE ${CMAKE_DL_LIBS})
    target_link_libraries(hooker_galore PRIVATE pthread)
    target_link_libraries(hooker_galore PRIVATE X11)
endif()

# Set up shared libraries location directory for MacOS.
if(APPLE)
    set_property(TARGET hooker_galore PROPERTY SKIP_BUILD_RPATH "ON")
    set_property(TARGET hooker_galore APPEND PROPERTY LINK_FLAGS "-Wl,-rpath,\"@executable_path\"")
endif()

# Compile shaders.

function(build_shaders TYPE GLOB_PATTERN)
    if(WIN32)
        set(shader_compiler "${CMAKE_SOURCE_DIR}/lib/bgfx/bin/Windows/shaderc.exe")
    elseif(APPLE)
        set(shader_compiler "${CMAKE_SOURCE_DIR}/lib/bgfx/bin/Darwin/shaderc")
    elseif(UNIX)
        set(shader_compiler "${CMAKE_SOURCE_DIR}/lib/bgfx/bin/Linux/shaderc")
    endif()

    if("${TYPE}" STREQUAL "vertex")
        set(t "v")
    else()
        set(t "p")
    endif()

    set(target_dependencies "")
    set(include_directory "${CMAKE_SOURCE_DIR}/src/shaders/headers/")

    file(GLOB_RECURSE SHADER_LIST ${GLOB_PATTERN})
    foreach(input ${SHADER_LIST})
        string(LENGTH "${input}" input_length)
        math(EXPR input_length_no_extension "${input_length} - 3")
        string(SUBSTRING "${input}" 0 ${input_length_no_extension} output)

        get_filename_component(file_name "${input}" NAME)
        string(LENGTH "${file_name}" file_name_length)
        math(EXPR file_name_length_no_extension "${file_name_length} - 3")
        string(SUBSTRING ${file_name} 0 ${file_name_length_no_extension} file_name_no_extension)
        string(REGEX REPLACE "[^A-Za-z0-9]" "_" variable_name "${file_name_no_extension}")

        if(WIN32)
            STRING(REGEX REPLACE "/" "\\\\" output ${output})

            add_custom_command(
                    DEPENDS "${input}"
                    OUTPUT "${output}.h"
                    COMMAND type nul > ${output}.h
                    COMMAND echo \#include ^<cstdint^>>>\"${output}.h\"
                    COMMAND echo. >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform linux -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_glsl
                    COMMAND copy /b \"${output}.h\"+\"${output}.temp.h\" \"${output}.h\" 1>NUL
                    COMMAND echo. >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform linux -p spirv -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_spv
                    COMMAND copy /b \"${output}.h\"+\"${output}.temp.h\" \"${output}.h\" 1>NUL
                    COMMAND echo. >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform windows -p ${t}s_3_0 -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_dx9
                    COMMAND copy /b \"${output}.h\"+\"${output}.temp.h\" \"${output}.h\" 1>NUL
                    COMMAND echo. >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform windows -p ${t}s_4_0 -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_dx11
                    COMMAND copy /b \"${output}.h\"+\"${output}.temp.h\" \"${output}.h\" 1>NUL
                    COMMAND echo. >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform osx -p metal -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_mtl
                    COMMAND copy /b \"${output}.h\"+\"${output}.temp.h\" \"${output}.h\" 1>NUL
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output}.temp.h\""
                    COMMENT "Building ${TYPE} shader ${input}"
            )
        elseif(APPLE)
            add_custom_command(
                    DEPENDS "${input}"
                    OUTPUT "${output}.h"
                    COMMAND cp /dev/null ${output}.h
                    COMMAND echo \"\#include <cstdint>\" >> \"${output}.h\"
                    COMMAND echo \"\" >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform linux -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_glsl
                    COMMAND cat \"${output}.temp.h\" >> \"${output}.h\"
                    COMMAND echo \"\" >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform linux -p spirv -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_spv
                    COMMAND cat \"${output}.temp.h\" >> \"${output}.h\"
                    COMMAND echo \"\" >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform osx -p metal -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_mtl
                    COMMAND cat \"${output}.temp.h\" >> \"${output}.h\"
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output}.temp.h\""
                    COMMENT "Building ${TYPE} shader ${input}"
            )
        elseif(UNIX)
            add_custom_command(
                    DEPENDS "${input}"
                    OUTPUT "${output}.h"
                    COMMAND cp /dev/null ${output}.h
                    COMMAND echo \"\#include <cstdint>\" >> \"${output}.h\"
                    COMMAND echo \"\" >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform linux -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_glsl
                    COMMAND cat \"${output}.temp.h\" >> \"${output}.h\"
                    COMMAND echo \"\" >> \"${output}.h\"
                    COMMAND ${shader_compiler} -i "\"${include_directory}\"" --type ${TYPE} --platform linux -p spirv -f "\"${input}\"" -o "\"${output}.temp.h\"" --bin2c ${variable_name}_spv
                    COMMAND cat \"${output}.temp.h\" >> \"${output}.h\"
                    COMMAND echo \"\" >> \"${output}.h\"
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output}.temp.h\""
                    COMMENT "Building ${TYPE} shader ${input}"
            )
        endif()

        list(APPEND target_dependencies "${output}.h")
    endforeach()

    add_custom_target("build_${TYPE}_shaders" ALL DEPENDS "${target_dependencies}")
endfunction()

build_shaders("fragment" "${CMAKE_SOURCE_DIR}/src/shaders/*.fragment.sc")
build_shaders("vertex" "${CMAKE_SOURCE_DIR}/src/shaders/*.vertex.sc")
add_dependencies(hooker_galore build_fragment_shaders build_vertex_shaders)

# Deploy shared libraries.

set(shared_libraries "")

if(WIN32)
    add_custom_command(
            DEPENDS "${CMAKE_SOURCE_DIR}/lib/SDL2/Windows/bin/SDL2.dll"
            OUTPUT "${CMAKE_BINARY_DIR}/SDL2.dll"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/lib/SDL2/Windows/bin/SDL2.dll" "${CMAKE_BINARY_DIR}/SDL2.dll"
            COMMENT "Deploying SDL2.dll"
    )
    list(APPEND shared_libraries "${CMAKE_BINARY_DIR}/SDL2.dll")

    if(MINGW AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        # Deploy "libgcc_s_seh-1.dll", "libstdc++-6.dll" and "libwinpthread-1.dll" in debug as well.
        get_filename_component(compiler_directory "${CMAKE_CXX_COMPILER}" DIRECTORY)

        if(EXISTS "${compiler_directory}/libgcc_s_seh-1.dll")
            add_custom_command(
                    DEPENDS "${compiler_directory}/libgcc_s_seh-1.dll"
                    OUTPUT "${CMAKE_BINARY_DIR}/libgcc_s_seh-1.dll"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${compiler_directory}/libgcc_s_seh-1.dll" "${CMAKE_BINARY_DIR}/libgcc_s_seh-1.dll"
                    COMMENT "Deploying libgcc_s_seh-1.dll"
            )
            list(APPEND shared_libraries "${CMAKE_BINARY_DIR}/libgcc_s_seh-1.dll")
        endif()

        if(EXISTS "${compiler_directory}/libstdc++-6.dll")
            add_custom_command(
                    DEPENDS "${compiler_directory}/libstdc++-6.dll"
                    OUTPUT "${CMAKE_BINARY_DIR}/libstdc++-6.dll"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${compiler_directory}/libstdc++-6.dll" "${CMAKE_BINARY_DIR}/libstdc++-6.dll"
                    COMMENT "Deploying libstdc++-6.dll"
            )
            list(APPEND shared_libraries "${CMAKE_BINARY_DIR}/libstdc++-6.dll")
        endif()

        if(EXISTS "${compiler_directory}/libwinpthread-1.dll")
            add_custom_command(
                    DEPENDS "${compiler_directory}/libwinpthread-1.dll"
                    OUTPUT "${CMAKE_BINARY_DIR}/libwinpthread-1.dll"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${compiler_directory}/libwinpthread-1.dll" "${CMAKE_BINARY_DIR}/libwinpthread-1.dll"
                    COMMENT "Deploying libwinpthread-1.dll"
            )
            list(APPEND shared_libraries "${CMAKE_BINARY_DIR}/libwinpthread-1.dll")
        endif()
    endif()
elseif(APPLE)
    add_custom_command(
            DEPENDS "${CMAKE_SOURCE_DIR}/lib/SDL2/Darwin/bin/SDL2.framework"
            OUTPUT "${CMAKE_BINARY_DIR}/SDL2.framework"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/lib/SDL2/Darwin/bin/SDL2.framework" "${CMAKE_BINARY_DIR}/SDL2.framework"
            COMMENT "Deploying SDL2.framework"
    )
    list(APPEND shared_libraries "${CMAKE_BINARY_DIR}/SDL2.framework")
elseif(UNIX)
    add_custom_command(
            DEPENDS "${CMAKE_SOURCE_DIR}/lib/SDL2/Linux/bin/libSDL2.so"
            OUTPUT "${CMAKE_BINARY_DIR}/libSDL2.so"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/lib/SDL2/Linux/bin/libSDL2.so" "${CMAKE_BINARY_DIR}/libSDL2.so"
            COMMENT "Deploying libSDL2.so"
    )
    list(APPEND shared_libraries "${CMAKE_BINARY_DIR}/libSDL2.so")
endif()

add_custom_target(deploy_shared_libraries ALL DEPENDS "${shared_libraries}")
add_dependencies(hooker_galore deploy_shared_libraries)

# Deploy resources.

function(deploy_resources TYPE GLOB_PATTERN OUT_DEPENDENCIES)
    set(dependencies ${${OUT_DEPENDENCIES}})
    string(LENGTH "${CMAKE_SOURCE_DIR}/res/" source_directory_length)

    file(GLOB_RECURSE FILE_LIST ${GLOB_PATTERN})
    foreach(input ${FILE_LIST})
        string(SUBSTRING ${input} ${source_directory_length} -1 input_relative)
        set(output "${CMAKE_BINARY_DIR}/resources/${input_relative}")

        file(TO_NATIVE_PATH ${input} input)
        file(TO_NATIVE_PATH ${output} output)

        add_custom_command(
                DEPENDS ${input}
                OUTPUT ${output}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${input} ${output}
                COMMENT "Deploying ${TYPE} ${input}"
        )

        list(APPEND dependencies ${output})
    endforeach()

    set(${OUT_DEPENDENCIES} ${dependencies} PARENT_SCOPE)
endfunction()

set(resources "")
deploy_resources("level" "${CMAKE_SOURCE_DIR}/res/levels/*.yaml" resources)
deploy_resources("material" "${CMAKE_SOURCE_DIR}/res/materials/*.yaml" resources)
deploy_resources("model" "${CMAKE_SOURCE_DIR}/res/models/*.glb" resources)
deploy_resources("preset" "${CMAKE_SOURCE_DIR}/res/presets/*.yaml" resources)
deploy_resources("texture" "${CMAKE_SOURCE_DIR}/res/textures/*.png" resources)
add_custom_target(deploy_resources ALL DEPENDS "${resources}")
add_dependencies(hooker_galore deploy_resources)
