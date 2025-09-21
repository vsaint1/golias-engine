cmake_minimum_required(VERSION 3.18)

project(engine_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
        test_hello.cpp

)

# TODO: enable this when more tests are added
#file(GLOB TEST_SOURCES "test_*.cpp")

function(add_gtest_executable test_src)
    get_filename_component(test_name ${test_src} NAME_WE)

    add_executable(${test_name} ${test_src})

    set_target_properties(${test_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
    )

    target_link_libraries(${test_name} PRIVATE engine GTest::gtest_main)


    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()


foreach(test_src IN LISTS TEST_SOURCES)
    add_gtest_executable(${test_src})
endforeach()
