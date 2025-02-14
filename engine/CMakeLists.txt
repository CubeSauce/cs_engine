add_subdirectory(third_party)

file(GLOB_RECURSE cs_engine_src
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
)

file(GLOB cs_engine_includes
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

message("${CMAKE_SOURCE_DIR}/engine/third_party/glew/include")

find_package(OpenGL REQUIRED)

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

message(ASAS ${OPENVR_LIB})

add_library(cs_engine)
target_sources(cs_engine PRIVATE ${cs_engine_src})
target_include_directories(cs_engine PUBLIC ${cs_engine_includes} 
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glew/include" 
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr/headers")
target_link_libraries(cs_engine PUBLIC glfw glew_s OpenGL::GL assimp 
 "C:\\Users\\Matija\\Documents\\GitHub\\cs_engine\\build\\lib\\Debug\\openvr_api64d.lib"
)
if (WIN32)
    target_link_libraries(cs_engine PUBLIC 
    d3d11.lib D3DCompiler.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    comdlg32.lib advapi32.lib shell32.lib
    ole32.lib oleaut32.lib)
elseif(APPLE)
    target_link_libraries(cs_engine PUBLIC "-framework Cocoa" "-framework IOKit")
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