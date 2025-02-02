add_subdirectory(third_party)

file(GLOB_RECURSE cs_engine_src
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
)

file(GLOB cs_engine_includes
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

message("${CMAKE_SOURCE_DIR}/engine/third_party/glew/include")

find_package(OpenGL REQUIRED)

add_library(cs_engine)
target_sources(cs_engine PRIVATE ${cs_engine_src})
target_include_directories(cs_engine PUBLIC ${cs_engine_includes} "${CMAKE_SOURCE_DIR}/engine/third_party/glew/include")
target_link_libraries(cs_engine PUBLIC glfw glew_s OpenGL::GL assimp
d3d11.lib D3DCompiler.lib dxgi.lib dxguid.lib uuid.lib
kernel32.lib user32.lib
comdlg32.lib advapi32.lib shell32.lib
ole32.lib oleaut32.lib)
