cmake_minimum_required(VERSION 3.18)

project(entt)

set(CMAKE_CXX_STANDARD 20)

add_library(entt INTERFACE)

target_include_directories(entt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
