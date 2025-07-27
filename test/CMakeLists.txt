include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/6230d316e104d61c5e08f0ef14ced638c1d4c760.zip)

FetchContent_MakeAvailable(googletest)

file(GLOB test_sources CONFIGURE_DEPENDS "*.cpp")

add_executable(${CMAKE_PROJECT_NAME}_test ${test_sources})
target_link_libraries(${CMAKE_PROJECT_NAME}_test PRIVATE ${CMAKE_PROJECT_NAME} GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${CMAKE_PROJECT_NAME}_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
