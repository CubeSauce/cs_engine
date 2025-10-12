file(GLOB cs_test_src
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
)

add_executable(cs_test)
target_sources(cs_test PRIVATE ${cs_test_src})
target_include_directories(cs_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(cs_test PUBLIC cs_engine)

add_custom_command(
        TARGET cs_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:cs_test>/assets
)

add_custom_command(
        TARGET cs_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CS_ENGINE_ASSETS}
        $<TARGET_FILE_DIR:cs_test>/assets
)