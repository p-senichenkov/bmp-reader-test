file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")

add_executable(${CMAKE_PROJECT_NAME} ${sources})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ".")
