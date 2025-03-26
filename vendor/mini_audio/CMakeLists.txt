cmake_minimum_required(VERSION 3.18)

project(mini_audio)

add_library(mini_audio)

target_sources(mini_audio PRIVATE src/miniaudio.c)

target_include_directories(mini_audio PUBLIC include)
