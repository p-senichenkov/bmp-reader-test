file(GLOB cli_sources CONFIGURE_DEPENDS "*.cpp")

add_executable(${CMAKE_PROJECT_NAME}_cli ${cli_sources})
target_link_libraries(${CMAKE_PROJECT_NAME}_cli ${CMAKE_PROJECT_NAME})
