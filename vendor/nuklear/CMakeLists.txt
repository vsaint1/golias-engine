cmake_minimum_required(VERSION 3.18)

project(nuklear)

set(CMAKE_CXX_STANDARD 11)

add_library(nuklear STATIC nuklear.cpp)

target_include_directories(nuklear PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nuklear PUBLIC SDL3::SDL3 glad)
