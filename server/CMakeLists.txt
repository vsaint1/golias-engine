cmake_minimum_required(VERSION 3.18)

project(ember_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)



if(NOT (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin"))
    message(WARNING "Server can only be built on Windows, Linux, or macOS")
    return()
endif()

file(GLOB_RECURSE SERVER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.c")


add_executable(${PROJECT_NAME} ${SERVER_SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PUBLIC engine)