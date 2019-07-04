# Build shaders of the specified type matching the specified GLOB pattern. Target "build_${TYPE}_shaders" is created.
# Sample usage: build_shaders("fragment" "${CMAKE_SOURCE_DIR}/shaders/*.fragment.sc" "${CMAKE_SOURCE_DIR}/shaders/headers/")
function(build_shaders TYPE GLOB_PATTERN INCLUDE_DIRECTORY)
    if(WIN32)
        set(shader_compiler "${CMAKE_SOURCE_DIR}/tools/Windows/shaderc.exe")
    else()
        set(shader_compiler "${CMAKE_SOURCE_DIR}/tools/${CMAKE_SYSTEM_NAME}/shaderc")
    endif()

    if("${TYPE}" STREQUAL "vertex")
        set(t "v")
    else()
        set(t "p")
    endif()

    set(target_dependencies "")

    file(GLOB_RECURSE SHADER_LIST ${GLOB_PATTERN})
    foreach(input ${SHADER_LIST})
        string(LENGTH "${input}" input_length)
        math(EXPR input_length_no_extension "${input_length} - 3")
        string(SUBSTRING "${input}" 0 ${input_length_no_extension} output_no_extension)
        set(output "${output_no_extension}.h")
        set(output_temp "${output_no_extension}.temp.h")

        file(TO_NATIVE_PATH "${input}" input)
        file(TO_NATIVE_PATH "${output}" output)
        file(TO_NATIVE_PATH "${output_temp}" output_temp)

        get_filename_component(file_name "${input}" NAME)
        string(LENGTH "${file_name}" file_name_length)
        math(EXPR file_name_length_no_extension "${file_name_length} - 3")
        string(SUBSTRING ${file_name} 0 ${file_name_length_no_extension} file_name_no_extension)
        string(REGEX REPLACE "[^A-Za-z0-9]" "_" variable_name "${file_name_no_extension}")

        if(WIN32)
            add_custom_command(
                    DEPENDS "${input}"
                    OUTPUT "${output}"
                    COMMAND type nul > ${output}
                    COMMAND echo \#include ^<cstdint^>>>\"${output}\"
                    COMMAND echo. >> \"${output}\"
                    COMMAND ${shader_compiler} -i "\"${INCLUDE_DIRECTORY}\"" --type ${TYPE} --platform linux --profile 150 -f "\"${input}\"" -o "\"${output_temp}\"" --bin2c ${variable_name}_glsl
                    COMMAND copy /b \"${output}\"+\"${output_temp}\" \"${output}\" 1>NUL
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output_temp}\""
                    COMMAND echo. >> \"${output}\"
                    COMMAND ${shader_compiler} -i "\"${INCLUDE_DIRECTORY}\"" --type ${TYPE} --platform linux -p spirv -f "\"${input}\"" -o "\"${output_temp}\"" --bin2c ${variable_name}_spv
                    COMMAND copy /b \"${output}\"+\"${output_temp}\" \"${output}\" 1>NUL
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output_temp}\""
                    COMMAND echo. >> \"${output}\"
                    COMMAND ${shader_compiler} -i "\"${INCLUDE_DIRECTORY}\"" --type ${TYPE} --platform windows -p ${t}s_3_0 -f "\"${input}\"" -o "\"${output_temp}\"" --bin2c ${variable_name}_dx9
                    COMMAND copy /b \"${output}\"+\"${output_temp}\" \"${output}\" 1>NUL
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output_temp}\""
                    COMMAND echo. >> \"${output}\"
                    COMMAND ${shader_compiler} -i "\"${INCLUDE_DIRECTORY}\"" --type ${TYPE} --platform windows -p ${t}s_4_0 -f "\"${input}\"" -o "\"${output_temp}\"" --bin2c ${variable_name}_dx11
                    COMMAND copy /b \"${output}\"+\"${output_temp}\" \"${output}\" 1>NUL
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output_temp}\""
                    COMMAND echo. >> \"${output}\"
                    COMMAND ${shader_compiler} -i "\"${INCLUDE_DIRECTORY}\"" --type ${TYPE} --platform osx -p metal -f "\"${input}\"" -o "\"${output_temp}\"" --bin2c ${variable_name}_mtl
                    COMMAND copy /b \"${output}\"+\"${output_temp}\" \"${output}\" 1>NUL
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output_temp}\""
                    COMMENT "Building ${TYPE} shader ${input}"
            )
        else()
            add_custom_command(
                    DEPENDS "${input}"
                    OUTPUT "${output}"
                    COMMAND cp /dev/null ${output}
                    COMMAND echo \"\#include <cstdint>\" >> \"${output}\"
                    COMMAND echo \"\" >> \"${output}\"
                    COMMAND ${shader_compiler} -i "\"${INCLUDE_DIRECTORY}\"" --type ${TYPE} --platform linux --profile 150 -f "\"${input}\"" -o "\"${output_temp}\"" --bin2c ${variable_name}_glsl
                    COMMAND cat \"${output_temp}\" >> \"${output}\"
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output_temp}\""
                    COMMAND echo \"\" >> \"${output}\"
                    COMMAND ${shader_compiler} -i "\"${INCLUDE_DIRECTORY}\"" --type ${TYPE} --platform linux -p spirv -f "\"${input}\"" -o "\"${output_temp}\"" --bin2c ${variable_name}_spv
                    COMMAND cat \"${output_temp}\" >> \"${output}\"
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output_temp}\""
                    COMMAND echo \"\" >> \"${output}\"
                    COMMAND ${shader_compiler} -i "\"${INCLUDE_DIRECTORY}\"" --type ${TYPE} --platform osx -p metal -f "\"${input}\"" -o "\"${output_temp}\"" --bin2c ${variable_name}_mtl
                    COMMAND cat \"${output_temp}\" >> \"${output}\"
                    COMMAND ${CMAKE_COMMAND} -E remove -f "\"${output_temp}\""
                    COMMENT "Building ${TYPE} shader ${input}"
            )
        endif()

        list(APPEND target_dependencies "${output}")
    endforeach()

    add_custom_target("build_${TYPE}_shaders" ALL DEPENDS "${target_dependencies}")
endfunction()
