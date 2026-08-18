cmake_minimum_required(VERSION 3.18)

project(tinyobjloader)

set(CMAKE_CXX_STANDARD 20)

add_library(tinyobjloader STATIC src/tiny_obj_loader.cpp)

target_include_directories(tinyobjloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
