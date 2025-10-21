cmake_minimum_required(VERSION 3.18)

project(engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

add_definitions(-DENGINE_VERSION="${ENGINE_VERSION}")

file(GLOB_RECURSE ENGINE_SOURCES "private/*.cpp" "private/*.c")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "engine/private/core/platform/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "engine/private/core/renderer/.mm")

add_library(engine STATIC ${ENGINE_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC vendor)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /bigobj )
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE "public/stdafx.h")