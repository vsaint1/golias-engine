cmake_minimum_required(VERSION 3.18)

project(mini_audio)

add_library(mini_audio STATIC src/stb_vorbis.c)

# MA [ios] must be obj-c
if (APPLE)
    message("Building mini_audio with objective-c")
    target_sources(mini_audio PRIVATE src/miniaudio.m)
else ()
    target_sources(mini_audio PRIVATE src/miniaudio.c)
endif ()


target_include_directories(mini_audio PUBLIC include)

