file(GLOB anim_test_src
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp"
)

add_executable(anim_test)
target_sources(anim_test PRIVATE ${anim_test_src})
target_include_directories(anim_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src2")
target_link_libraries(anim_test PUBLIC cs_engine)