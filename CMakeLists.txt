cmake_minimum_required(VERSION 3.5)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(CS)
set (CMAKE_CXX_STANDARD 20)

add_subdirectory(engine)
add_subdirectory(apps)
