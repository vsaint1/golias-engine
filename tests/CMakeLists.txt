cmake_minimum_required(VERSION 3.20)

project(golias_tests)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(golias_tests
        test_strings.cpp
        test_math.cpp
)

target_link_libraries(golias_tests PRIVATE
        GTest::gtest_main
        engine
)

target_include_directories(golias_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/engine
)

target_compile_definitions(golias_tests PRIVATE
        BUILD_GOLIAS_TESTS
)


add_custom_command(TARGET golias_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:golias_tests>/res
)


include(GoogleTest)
gtest_discover_tests(golias_tests)
