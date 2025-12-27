cmake_minimum_required(VERSION 3.18)

project(tinyobjloader)

set(CMAKE_CXX_STANDARD 20)

add_library(tinyobjloader INTERFACE)

target_include_directories(tinyobjloader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
