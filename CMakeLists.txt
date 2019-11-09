cmake_minimum_required(VERSION 3.10)
project(hooker_galore)

include(cmake/shaderc.cmake)
include(cmake/texturec.cmake)
include(cmake/deploy.cmake)

# Compile small thirdpary libraries in place.

add_subdirectory("${CMAKE_SOURCE_DIR}/thirdparty/imgui/")
add_subdirectory("${CMAKE_SOURCE_DIR}/thirdparty/debug-draw/")
add_subdirectory("${CMAKE_SOURCE_DIR}/thirdparty/tiny-gltf/")

# Select C++ version and set up visual studio solution properties.

set(CMAKE_CXX_STANDARD 17)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT hooker_galore)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "CustomTargets")

# Compile executable sources.

file(GLOB_RECURSE HOOKER_GALORE_SOURCES "${CMAKE_SOURCE_DIR}/sources/*.cpp" "${CMAKE_SOURCE_DIR}/sources/*.h" "${CMAKE_SOURCE_DIR}/sources/*.sc" "${CMAKE_SOURCE_DIR}/sources/*.sh")
add_executable(hooker_galore ${HOOKER_GALORE_SOURCES})

# Use filesystem hierarchy in Visual Studio project.

if(WIN32)
    foreach(FILE ${HOOKER_GALORE_SOURCES}) 
        get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
        string(REPLACE "${CMAKE_SOURCE_DIR}/sources" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        source_group("Sources${GROUP}" FILES "${FILE}")
    endforeach()
endif()

# Include thirdparty libraries.

function(include_source_directory TARGET DIRECTORY)
	target_include_directories(hooker_galore PRIVATE ${DIRECTORY})
	
	# Add thirdparty headers to Visual Studio as well.
	if(WIN32)
		file(GLOB_RECURSE TARGET_SOURCES "${DIRECTORY}/*.h" "${DIRECTORY}/*.hpp")
		
		add_library(${TARGET} STATIC EXCLUDE_FROM_ALL ${TARGET_SOURCES})
		set_target_properties(${TARGET} PROPERTIES 
				LINKER_LANGUAGE CXX
				FOLDER "BuildAndDeploy"
		)
		
		foreach(FILE ${TARGET_SOURCES}) 
			get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
			string(REPLACE "${DIRECTORY}" "" GROUP "${PARENT_DIR}")
			string(REPLACE "/" "\\" GROUP "${GROUP}")
			source_group("Header Files${GROUP}" FILES "${FILE}")
		endforeach()
	endif()
endfunction()

target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/sources/")
include_source_directory(bgfx "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/include")
include_source_directory(SDL2 "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/${CMAKE_HOST_SYSTEM_NAME}/include")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/imgui/include")
include_source_directory(Clara "${CMAKE_SOURCE_DIR}/thirdparty/Clara/include")
include_source_directory(yaml-cpp "${CMAKE_SOURCE_DIR}/thirdparty/yaml-cpp/include")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/debug-draw/include")
include_source_directory(entt "${CMAKE_SOURCE_DIR}/thirdparty/entt/include")
include_source_directory(glm "${CMAKE_SOURCE_DIR}/thirdparty/glm/include")
include_source_directory(libfmt "${CMAKE_SOURCE_DIR}/thirdparty/libfmt/include")
target_include_directories(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/tiny-gltf/include")
include_source_directory(ghc-filesystem "${CMAKE_SOURCE_DIR}/thirdparty/ghc-filesystem/include")
include_source_directory(PhysX "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/include")

# Link thirdpary libraries.

target_link_libraries(hooker_galore PRIVATE imgui)
target_link_libraries(hooker_galore PRIVATE debug-draw)
target_link_libraries(hooker_galore PRIVATE tiny-gltf)

if(WIN32)
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/Windows/lib/SDL2main.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/Windows/lib/SDL2.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/bgfx.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/bimg.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/bx.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/astc-codec.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/yaml-cpp/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/yaml-cpp.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/libfmt/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/fmt.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysX_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXCharacterKinematic_static_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXCooking_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXExtensions_static_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXVehicle_static_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/SimulationController_static_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXFoundation_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/LowLevel_static_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/LowLevelAABB_static_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/LowLevelDynamics_static_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXCommon_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/SceneQuery_static_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXTask_static_64.lib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXPvdSDK_static_64.lib")
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(METAL_LIBRARY Metal)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    target_link_libraries(hooker_galore PRIVATE "${COCOA_LIBRARY}" "${METAL_LIBRARY}" "${QUARTZCORE_LIBRARY}")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/Darwin/bin/libSDL2.dylib")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libbgfx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libbimg.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libbx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libastc-codec.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/yaml-cpp/lib/Darwin/libyaml-cpp.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/libfmt/lib/Darwin/libfmt.a")
    # TODO: PhysX
elseif(UNIX)
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/Linux/bin/libSDL2.so")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libbgfx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libbimg.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libbx.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/bgfx/lib/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libastc-codec.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/yaml-cpp/lib/Linux/libyaml-cpp.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/libfmt/lib/Linux/libfmt.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysX_64.so")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXCharacterKinematic_static_64.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXCooking_64.so")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXExtensions_static_64.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXVehicle_static_64.a")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXFoundation_64.so")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXCommon_64.so")
    target_link_libraries(hooker_galore PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/lib/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXPvdSDK_static_64.a")
    target_link_libraries(hooker_galore PRIVATE X11 pthread ${CMAKE_DL_LIBS} ${OPENGL_LIBRARIES})
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

if(UNIX)
    set_property(TARGET hooker_galore PROPERTY SKIP_BUILD_RPATH "ON")

    if(APPLE)
        set_property(TARGET hooker_galore APPEND PROPERTY LINK_FLAGS "-Wl,-rpath,\"@executable_path\"")
    else()
        set_property(TARGET hooker_galore APPEND PROPERTY LINK_FLAGS "-Wl,-rpath,\"$ORIGIN\"")
    endif()
endif()

# Add required compile definitions.

if(WIN32)
    target_compile_definitions(hooker_galore PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(UNIX)
    target_compile_definitions(hooker_galore PRIVATE _DEBUG)
endif()

# Compile shaders.

build_shaders("fragment" "${CMAKE_SOURCE_DIR}/sources/shaders/*.fragment.sc" "${CMAKE_SOURCE_DIR}/sources/shaders/headers/")
build_shaders("vertex" "${CMAKE_SOURCE_DIR}/sources/shaders/*.vertex.sc" "${CMAKE_SOURCE_DIR}/sources/shaders/headers/")
add_dependencies(hooker_galore build_fragment_shaders build_vertex_shaders)

# Compile textures.

set(output_directory "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/")
set(output_resources_directory "${output_directory}/resources/")
set(output_textures_directory "${output_resources_directory}/textures/")

build_textures("base_color_roughness" "${CMAKE_SOURCE_DIR}/resources/textures/" "*_bcr.png" "${output_textures_directory}")
build_textures("normal_metal_ambient_occlusion" "${CMAKE_SOURCE_DIR}/resources/textures/" "*_nmao.png" "${output_textures_directory}")
build_textures("height" "${CMAKE_SOURCE_DIR}/resources/textures/" "*_h.png" "${output_textures_directory}")
build_textures("cube" "${CMAKE_SOURCE_DIR}/resources/textures/" "*.hdr" "${output_textures_directory}")
add_dependencies(hooker_galore
        build_base_color_roughness_maps
        build_normal_metal_ambient_occlusion_maps
        build_height_maps
        build_cube_maps
)

# Deploy other resources.

deploy_resources("level" "${CMAKE_SOURCE_DIR}/resources/levels/" "*.yaml" "${output_resources_directory}/levels/")
deploy_resources("model" "${CMAKE_SOURCE_DIR}/resources/models/" "*.glb" "${output_resources_directory}/models/")
deploy_resources("preset" "${CMAKE_SOURCE_DIR}/resources/presets/" "*.yaml" "${output_resources_directory}/presets/")
deploy_resources("texture" "${CMAKE_SOURCE_DIR}/resources/textures/" "*.dds" "${output_resources_directory}/textures/")
add_dependencies(hooker_galore
        deploy_levels
        deploy_models
        deploy_presets
        deploy_textures
)

# Deploy shared libraries.

deploy_shared_library("SDL2"
        "${output_directory}"
        "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/Windows/bin/SDL2.dll"
        "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/Darwin/bin/libSDL2.dylib"
        "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/Linux/bin/libSDL2.so"
)
deploy_shared_library("PhysX_64"
        "${output_directory}"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysX_64.dll"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysX_64.dylib"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysX_64.so"
)
deploy_shared_library("PhysXCommon_64"
        "${output_directory}"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXCommon_64.dll"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXCommon_64.dylib"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXCommon_64.so"
)
deploy_shared_library("PhysXCooking_64"
        "${output_directory}"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXCooking_64.dll"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXCooking_64.dylib"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXCooking_64.so"
)
deploy_shared_library("PhysXFoundation_64"
        "${output_directory}"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Windows/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/PhysXFoundation_64.dll"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Darwin/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXFoundation_64.dylib"
        "${CMAKE_SOURCE_DIR}/thirdparty/PhysX/bin/Linux/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/libPhysXFoundation_64.so"
)
add_dependencies(hooker_galore 
        deploy_SDL2
        deploy_PhysX_64
        deploy_PhysXCommon_64
        deploy_PhysXCooking_64
        deploy_PhysXFoundation_64
)

# Hide all the deploy targets into a "Deploy" directory in Visual Studio.

set_target_properties(imgui
        debug-draw
        tiny-gltf
        build_fragment_shaders
        build_vertex_shaders
        build_base_color_roughness_maps
        build_normal_metal_ambient_occlusion_maps
        build_height_maps
        build_cube_maps
        deploy_levels
        deploy_models
        deploy_presets
        deploy_textures
        deploy_SDL2
        deploy_PhysX_64
        deploy_PhysXCommon_64
        deploy_PhysXCooking_64
        deploy_PhysXFoundation_64
        PROPERTIES FOLDER "BuildAndDeploy"
)
