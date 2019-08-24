# Convert and compress textures of the specified type from the given directory to output directory. Target "build_${TYPE}_maps" is created.
# Usage example: build_texturesbuild_textures("base_color_roughness" "${CMAKE_SOURCE_DIR}/resources/textures/" "*_bcr.png" "${CMAKE_BINARY_DIR}/resources/textures/")
function(build_textures TYPE INPUT_DIRECTORY EXTENSION_PATTERN OUTPUT_DIRECTORY)
    string(LENGTH "${INPUT_DIRECTORY}" input_directory_length)

    if(WIN32)
        set(texture_compiler "${CMAKE_SOURCE_DIR}/tools/Windows/texturec.exe")
    else()
        set(texture_compiler "${CMAKE_SOURCE_DIR}/tools/${CMAKE_SYSTEM_NAME}/texturec")
    endif()

    set(compiler_flag "--albedo-roughness")
    set(is_cubemap OFF)
    if("${TYPE}" STREQUAL "normal_metal_ambient_occlusion")
        set(compiler_flag "--normal-metalness-ambient-occlusion")
    elseif("${TYPE}" STREQUAL "height")
        set(compiler_flag "--parallax")
    elseif("${TYPE}" STREQUAL "cube")
        set(compiler_flag "--cube-map")
        set(is_cubemap ON)
    endif()

    set(quality_preset "--development")
    if(QUALITY_NO_COMPRESSION)
        set(quality_preset "--no-compression")
    elseif(QUALITY_PRODUCTION)
        set(quality_preset "--production")
    endif()

    set(target_dependencies "")

    file(GLOB_RECURSE TEXTURE_LIST "${INPUT_DIRECTORY}/${EXTENSION_PATTERN}")
    foreach(input ${TEXTURE_LIST})
        string(SUBSTRING "${input}" "${input_directory_length}" -1 input_relative)
        string(LENGTH "${input_relative}" input_relative_length)
        math(EXPR input_relative_length_no_extension "${input_relative_length} - 4")
        string(SUBSTRING "${input_relative}" 0 ${input_relative_length_no_extension} output_relative_no_extension)
        set(output_no_extension "${OUTPUT_DIRECTORY}/${output_relative_no_extension}")


        set(cube_map_options "")
        if(${is_cubemap})
            set(output_irradiance "${output_no_extension}_irradiance.dds")
            file(TO_NATIVE_PATH "${output_irradiance}" output_irradiance)

            set(output_prefilter "${output_no_extension}_prefilter.dds")
            file(TO_NATIVE_PATH "${output_prefilter}" output_prefilter)

            separate_arguments(cube_map_options NATIVE_COMMAND "--output-size 1024 --irradiance ${output_irradiance} --irradiance-size 64 --prefilter ${output_prefilter} --prefilter-size 128")
        endif()

        file(TO_NATIVE_PATH "${input}" input)

        set(output "${output_no_extension}.dds")
        file(TO_NATIVE_PATH "${output}" output)

        get_filename_component(output_directory "${output}" DIRECTORY)

        add_custom_command(
                DEPENDS "${input}"
                OUTPUT "${output}"
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${output_directory}"
                COMMAND ${texture_compiler} "${compiler_flag}" --input "${input}" --output "${output}" ${cube_map_options} "${quality_preset}"
                COMMENT "Compiling ${TYPE} texture ${input}"
        )

        list(APPEND target_dependencies "${output}")
    endforeach()

    add_custom_target("build_${TYPE}_maps" ALL DEPENDS "${target_dependencies}")
endfunction()
