cmake_minimum_required(VERSION 3.16)
project(stb_truetype)

add_library(stb_truetype STATIC "src/stb_truetype.cpp")

target_include_directories(stb_truetype PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

