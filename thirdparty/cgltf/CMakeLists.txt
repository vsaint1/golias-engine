cmake_minimum_required(VERSION 3.18)

project(cgltf)

set(CMAKE_C_STANDARD 99)

add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
