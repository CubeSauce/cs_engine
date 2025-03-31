# For macos restriction
cmake_minimum_required(VERSION 3.5...4.5)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(cs_enviroment VERSION 0.0.1 LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 20)

add_subdirectory(engine)
add_subdirectory(apps)
