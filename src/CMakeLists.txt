file(GLOB_RECURSE lib_sources CONFIGURE_DEPENDS "*.cpp")

add_library(${CMAKE_PROJECT_NAME} ${lib_sources})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ".")
