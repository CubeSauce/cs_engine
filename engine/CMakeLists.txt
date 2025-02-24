add_subdirectory(third_party)

file(GLOB_RECURSE cs_engine_src
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
)

file(GLOB cs_engine_includes
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
)

option(VR_SUPPORT "Should use VR" OFF)
add_compile_definitions(WITH_VR_SUPPORT=${VR_SUPPORT})

message("${CMAKE_SOURCE_DIR}/engine/third_party/glew/include")

find_package(OpenGL REQUIRED)

if(VR_SUPPORT)
set(SIZEOF_VOIDP ${CMAKE_SIZEOF_VOID_P})
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PROCESSOR_ARCH "64")
else()
    set(PROCESSOR_ARCH "32")
endif()

if(WIN32)
    set(PLATFORM_NAME "win")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_NAME "linux")
elseif(APPLE)
    set(PLATFORM_NAME "osx")
endif()

find_library(OPENVR_LIB openvr_api64d HINTS 
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr/bin/${PLATFORM_NAME}${PROCESSOR_ARCH}" 
    NO_DEFAULT_PATH )
endif()
message(ASAS ${OPENVR_LIB})

add_library(cs_engine)
target_sources(cs_engine PRIVATE ${cs_engine_src})
target_include_directories(cs_engine PUBLIC ${cs_engine_includes} 
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glew/include" 
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr/headers")
target_link_libraries(cs_engine PUBLIC glfw glew_s OpenGL::GL assimp 
)
if (WIN32)
    target_link_libraries(cs_engine PUBLIC 
    d3d11.lib D3DCompiler.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    comdlg32.lib advapi32.lib shell32.lib
    ole32.lib oleaut32.lib
    "C:\\Users\\Matija\\Documents\\GitHub\\cs_engine\\build\\lib\\Debug\\openvr_api64d.lib"
    )
elseif(APPLE)
    target_link_libraries(cs_engine PUBLIC "-framework Cocoa" "-framework IOKit" "-framework AppKit")
    if(VR_SUPPORT)
    target_link_libraries(cs_engine PUBLIC 
        "/Users/cubesauce/Projects/cs_engine/engine/third_party/openvr/bin/osx32/libopenvr_api.dylib"
    )
    endif()
endif (WIN32)
set_property(TARGET cs_engine PROPERTY CXX_STANDARD 20)

# Copy driver assets to output folder
add_custom_command(
    TARGET cs_engine
    PRE_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_SOURCE_DIR}/engine/assets/ 
    $<TARGET_FILE_DIR:cs_engine>
)

add_custom_command(
    TARGET cs_engine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    $<TARGET_FILE:cs_engine>
    $<TARGET_FILE_DIR:cs_engine>/bin/${PLATFORM_NAME}${PROCESSOR_ARCH}/$<TARGET_FILE_NAME:cs_engine>
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
