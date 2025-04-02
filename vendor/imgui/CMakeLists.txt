cmake_minimum_required(VERSION 3.18)

project(imgui)

file(GLOB_RECURSE IMGUI_SOURCES "*.cpp")

add_library(imgui STATIC ${IMGUI_SOURCES})

target_link_libraries(imgui SDL3::SDL3)

target_include_directories(imgui PUBLIC include)
