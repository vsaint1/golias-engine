cmake_minimum_required(VERSION 3.18)

project(flecs)

set(CMAKE_C_STANDARD 99)


add_library(flecs STATIC flecs.c flecs.h)

add_compile_definitions(-Dflecs_EXPORTS)

target_include_directories(flecs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
