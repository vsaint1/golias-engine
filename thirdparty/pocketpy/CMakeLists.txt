cmake_minimum_required(VERSION 3.18)

add_library(pocketpy STATIC src/pocketpy.c)
target_include_directories(pocketpy PUBLIC include)
target_compile_features(pocketpy PRIVATE c_std_11)

if(MSVC)
        target_compile_options(pocketpy PRIVATE /std:c11)
        target_compile_options(pocketpy PRIVATE /utf-8)
        target_compile_options(pocketpy PRIVATE /experimental:c11atomics)
        target_link_libraries(pocketpy PUBLIC ws2_32)
endif()


