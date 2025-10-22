cmake_minimum_required(VERSION 3.18)

project(flecs)

set(CMAKE_C_STANDARD 99)


add_library(flecs STATIC flecs.c flecs.h)

target_compile_definitions(flecs PRIVATE flecs_EXPORTS)

target_include_directories(flecs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
