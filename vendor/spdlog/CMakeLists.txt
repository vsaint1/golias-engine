cmake_minimum_required(VERSION 3.18)

project(spdlog LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(spdlog INTERFACE)

target_include_directories(spdlog INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(spdlog INTERFACE /utf-8)
endif()

find_package(Threads REQUIRED)

target_link_libraries(spdlog INTERFACE Threads::Threads)

if(UNIX)
    target_compile_options(spdlog INTERFACE -fPIC)
endif()
