cmake_minimum_required(VERSION 3.5...4.5)

project(cs_engine VERSION 0.0.2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

set(GLFW_VERSION "3.4")
set(GLEW_VERSION "2.2.0")
set(ASSIMP_VERSION "6.0.2")
set(OPENVR_VERSION "2.5.1")
set(GNS_VERSION "1.4.1")

FetchContent_Declare(
        glfw
        URL "https://github.com/glfw/glfw/archive/refs/tags/${GLFW_VERSION}.zip"
)

FetchContent_Declare(
        glew
        URL "https://github.com/nigels-com/glew/releases/download/glew-${GLEW_VERSION}/glew-${GLEW_VERSION}.zip"
)

FetchContent_Declare(
        stb
        GIT_REPOSITORY "https://github.com/nothings/stb.git"
)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v${ASSIMP_VERSION}
)

FetchContent_Declare(
        openvr
        URL "https://github.com/ValveSoftware/openvr/archive/refs/tags/v${OPENVR_VERSION}.zip"
)

# Use VCPKG to download OpenSSL and Protobuf dev libs.
FetchContent_Declare(
        GameNetworkingSockets
        GIT_REPOSITORY https://github.com/ValveSoftware/GameNetworkingSockets.git
        GIT_TAG v${GNS_VERSION}
)

FetchContent_MakeAvailable(
        glfw glew stb assimp openvr GameNetworkingSockets
)

find_package(OpenGL REQUIRED)

if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
endif()

add_subdirectory(${glew_SOURCE_DIR}/build/cmake ${glew_SOURCE_DIR})

# OpenVR
if(PLATFORM_WINDOWS)
    set(OPENVR_PLATFORM "win64")
    set(OPENVR_LIB_SUFFIX ".lib")
    set(OPENVR_BIN_SUFFIX ".dll")
elseif(PLATFORM_LINUX)
    set(OPENVR_PLATFORM "linux64")
    set(OPENVR_LIB_SUFFIX ".so")
    set(OPENVR_BIN_SUFFIX ".so")
elseif(PLATFORM_MACOS)
    set(OPENVR_PLATFORM "osx32")
    set(OPENVR_LIB_SUFFIX ".dylib")
    set(OPENVR_BIN_SUFFIX ".dylib")
endif()

add_library(cs_openvr SHARED IMPORTED)
set_target_properties(cs_openvr PROPERTIES
        IMPORTED_LOCATION "${openvr_SOURCE_DIR}/bin/${OPENVR_PLATFORM}/openvr_api${OPENVR_BIN_SUFFIX}"
        IMPORTED_IMPLIB "${openvr_SOURCE_DIR}/lib/${OPENVR_PLATFORM}/openvr_api${OPENVR_LIB_SUFFIX}"
        INTERFACE_INCLUDE_DIRECTORIES "${openvr_SOURCE_DIR}/headers"
)
# OpenVR

file(GLOB_RECURSE ${PROJECT_NAME}_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp")

set(CS_ENGINE_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(CS_ENGINE_ASSETS ${CS_ENGINE_ASSETS_DIR} PARENT_SCOPE)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_sources})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glew_s assimp::assimp cs_openvr GameNetworkingSockets)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    PRIVATE
        ${stb_SOURCE_DIR}
        ${glew_SOURCE_DIR}/include
)

if (PLATFORM_WINDOWS)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            d3d11.lib D3DCompiler.lib dxgi.lib oleaut32.lib
            dxguid.lib uuid.lib kernel32.lib user32.lib
            comdlg32.lib advapi32.lib shell32.lib ole32.lib
            OpenGL::GL
    )
elseif (PLATFORM_APPLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        "-framework Cocoa" "-framework Carbon" "-framework IOKit" "-framework AppKit" "-framework OpenGL"
    )
endif (PLATFORM_WINDOWS)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${PROJECT_NAME}>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/bin/${PLATFORM_NAME}${PROCESSOR_ARCH}/$<TARGET_FILE_NAME:cs_engine>
)

if (cs_shaders)
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
endif()