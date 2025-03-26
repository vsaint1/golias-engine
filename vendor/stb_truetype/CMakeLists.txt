cmake_minimum_required(VERSION 3.18)

project(stb_truetype)

add_library(stb_truetype STATIC)

target_sources(stb_truetype PRIVATE src/stb_truetype.cpp)

target_include_directories(stb_truetype PUBLIC include)
