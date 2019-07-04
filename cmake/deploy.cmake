# Deploy files from the specified input directory and given extension to provided output directory. Target "deploy_${TYPE}s" is created.
# Usage example: deploy_resources("textures" "${CMAKE_SOURCE_DIR}/resources/textures/" "*.png" "${CMAKE_BINARY_DIR}/resources/textures/")
function(deploy_resources TYPE INPUT_DIRECTORY EXTENSION_PATTERN OUTPUT_DIRECTORY)
    string(LENGTH "${INPUT_DIRECTORY}" input_directory_length)

    set(target_dependencies "")

    file(GLOB_RECURSE RESOURCE_LIST "${INPUT_DIRECTORY}/${EXTENSION_PATTERN}")
    foreach(input ${RESOURCE_LIST})
        string(SUBSTRING "${input}" "${input_directory_length}" -1 input_relative)
        set(output "${OUTPUT_DIRECTORY}/${input_relative}")

        file(TO_NATIVE_PATH "${input}" input)
        file(TO_NATIVE_PATH "${output}" output)

        add_custom_command(
                DEPENDS "${input}"
                OUTPUT "${output}"
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${input}" "${output}"
                COMMENT "Deploying ${TYPE} ${input}"
        )

        list(APPEND target_dependencies "${output}")
    endforeach()

    add_custom_target("deploy_${TYPE}s" ALL DEPENDS "${target_dependencies}")
endfunction()

# Deploy shared library of current platform to specified output directory. Target "deploy_${NAME}" is created.
# Usage example: deploy_shared_library("sdl2"
#                       "${CMAKE_BINARY_DIR}"
#                       "${CMAKE_SOURCE_DIR}/Windows/SDL2.dll"
#                       "${CMAKE_SOURCE_DIR}/Darwin/SDL2.dylib"
#                       "${CMAKE_SOURCE_DIR}/Linux/SDL2.so"
#                )
function(deploy_shared_library NAME OUTPUT_DIRECTORY WINDOWS_PATH DARWIN_PATH LINUX_PATH)
    set(input "")
    if(WIN32)
        set(input "${WINDOWS_PATH}")
    elseif(APPLE)
        set(input "${DARWIN_PATH}")
    else()
        set(input "${LINUX_PATH}")
    endif()

    get_filename_component(file_name "${input}" NAME)
    set(output "${OUTPUT_DIRECTORY}/${file_name}")

    file(TO_NATIVE_PATH "${input}" input)
    file(TO_NATIVE_PATH "${output}" output)

    add_custom_command(
            DEPENDS "${input}"
            OUTPUT "${output}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${input}" "${output}"
            COMMENT "Deploying ${NAME}"
    )

    add_custom_target("deploy_${NAME}" ALL DEPENDS "${output}")
endfunction()
