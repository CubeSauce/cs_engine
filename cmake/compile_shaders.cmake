function(compile_hlsl_to_spirv HLSL_SHADERS HLSL_TYPE SPIRV_OUTPUT_DIR SPIRV_OUTPUTS)
    find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)

    set(SPIRV_FILES "")
    foreach(HLSL_FILE ${HLSL_SHADERS})
        get_filename_component(SHADER_NAME ${HLSL_FILE} NAME_WE)
        set(SPIRV_FILE "${SPIRV_OUTPUT_DIR}/${SHADER_NAME}_${HLSL_TYPE}.spv")

        add_custom_command(
            OUTPUT ${SPIRV_FILE}
            COMMAND ${GLSLANG_VALIDATOR} -V -D ${HLSL_FILE} -e "main" -o ${SPIRV_FILE}
            DEPENDS ${HLSL_FILE}
            COMMENT "Compiling ${HLSL_FILE} to ${SPIRV_FILE}..."
        )

        list(APPEND SPIRV_FILES ${SPIRV_FILE})
    endforeach()

    set(${SPIRV_OUTPUTS} ${SPIRV_FILES} PARENT_SCOPE)
endfunction()

function(convert_spirv_to_glsl SPIRV_SHADERS GLSL_OUTPUT_DIR GLSL_OUTPUTS)
    find_program(SPIRV_CROSS spirv-cross REQUIRED)

    set(GLSL_FILES "")
    foreach(SPIRV_FILE ${SPIRV_SHADERS})
        get_filename_component(SHADER_NAME ${SPIRV_FILE} NAME_WE)
        set(GLSL_FILE "${GLSL_OUTPUT_DIR}/${SHADER_NAME}.glsl")

        add_custom_command(
            OUTPUT ${GLSL_FILE}
            COMMAND ${SPIRV_CROSS} ${SPIRV_FILE} --output ${GLSL_FILE} --flatten-ubo
            DEPENDS ${SPIRV_FILE}
            COMMENT "Converting ${SPIRV_FILE} to ${GLSL_FILE}..."
        )

        list(APPEND GLSL_FILES ${GLSL_FILE})
    endforeach()

    set(${GLSL_OUTPUTS} ${GLSL_FILES} PARENT_SCOPE)
endfunction()
