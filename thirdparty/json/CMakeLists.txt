cmake_minimum_required(VERSION 3.18)

project(json)

set(CMAKE_CXX_STANDARD 20)

add_library(json INTERFACE)

target_include_directories(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
