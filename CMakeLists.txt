cmake_minimum_required(VERSION 3.24)

project(BMPReader CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For Ninja to properly display colored output
set(CMAKE_COLOR_DIAGNOSTICS ON)

option(BUILD_TESTS "Compile tests" ON)
option(BUILD_CLI "Compile command-line interface" ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/target")

add_subdirectory("src")

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory("test")
endif()

if(BUILD_CLI)
  add_subdirectory("cli")
endif()
