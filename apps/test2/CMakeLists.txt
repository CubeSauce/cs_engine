file(GLOB cs_test2_src
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
)

add_executable(cs_test2)
target_sources(cs_test2 PRIVATE ${cs_test2_src})
target_include_directories(cs_test2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(cs_test2 PUBLIC cs_engine)