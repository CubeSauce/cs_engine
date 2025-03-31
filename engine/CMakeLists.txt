cmake_minimum_required(VERSION 3.5...4.5)

project(cs_engine VERSION 0.0.2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

option(cs_vr_support "Should use VR" OFF)

file(GLOB_RECURSE ${PROJECT_NAME}_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_sources})

find_package(OpenGL REQUIRED)
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/assimp)
add_subdirectory(third_party/glew/build/cmake)
if (VR_SUPPORT)
    add_subdirectory(third_party/openvr)
    target_add_compile_definitions(${PROJECT_NAME} PUBLIC CS_WITH_VR_SUPPORT)
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    PRIVATE
        glfw
        third_party
        third_party/glew/include
        third_party/assimp/include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        
    PRIVATE
        glfw
        glew
        assimp
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} 
        PUBLIC 
            d3d11.lib D3DCompiler.lib dxgi.lib oleaut32.lib 
            dxguid.lib uuid.lib kernel32.lib user32.lib 
            comdlg32.lib advapi32.lib shell32.lib ole32.lib 
            OpenGL::GL 
            #"C:\\Users\\Matija\\Documents\\GitHub\\cs_engine\\build\\lib\\Debug\\openvr_api64d.lib"
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} 
        PUBLIC 
            "-framework Cocoa" "-framework Carbon" "-framework IOKit" 
            "-framework AppKit" "-framework OpenGL"
    )            
endif (WIN32)

# Copy driver assets to output folder
add_custom_command(
    TARGET ${PROJECT_NAME} 
    PRE_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_SOURCE_DIR}/engine/assets/ 
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(
    TARGET ${PROJECT_NAME} 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    $<TARGET_FILE:${PROJECT_NAME}>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/bin/${PLATFORM_NAME}${PROCESSOR_ARCH}/$<TARGET_FILE_NAME:cs_engine>
)

include("${CMAKE_SOURCE_DIR}/cmake/compile_shaders.cmake")

file(GLOB HLSL_VERT_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/**.hlsl.vert")
compile_hlsl_to_spirv("${HLSL_VERT_SHADERS}" "vert" 
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/gen/spirv" SPIRV_VERT_SHADERS)

set(GLSL_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/gen/glsl")
convert_spirv_to_glsl("${SPIRV_VERT_SHADERS}" "${GLSL_OUTPUT_DIR}" GLSL_VERT_SHADERS)

file(GLOB HLSL_FRAG_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/**.hlsl.frag")
compile_hlsl_to_spirv("${HLSL_FRAG_SHADERS}" "frag" 
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/gen/spirv" SPIRV_FRAG_SHADERS)

set(GLSL_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/gen/glsl")
convert_spirv_to_glsl("${SPIRV_FRAG_SHADERS}" "${GLSL_OUTPUT_DIR}" GLSL_FRAG_SHADERS)

# Define build targets
add_custom_target(compile_shaders_vert ALL DEPENDS ${SPIRV_VERT_SHADERS})
add_custom_target(compile_shaders_frag ALL DEPENDS ${SPIRV_FRAG_SHADERS})
add_custom_target(convert_shaders_vert ALL DEPENDS ${GLSL_VERT_SHADERS})
add_custom_target(convert_shaders_frag ALL DEPENDS ${GLSL_FRAG_SHADERS})

# Ensure GLSL conversion happens after HLSL compilation
add_dependencies(convert_shaders_vert compile_shaders_vert)
add_dependencies(convert_shaders_frag compile_shaders_frag)
