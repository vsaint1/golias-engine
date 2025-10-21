cmake_minimum_required(VERSION 3.18)

project(lua)

file(GLOB LUA_SOURCES "*.c")
list(FILTER LUA_SOURCES EXCLUDE REGEX "lua.c")
list(FILTER LUA_SOURCES EXCLUDE REGEX "onelua.c")
set(CMAKE_CXX_STANDARD 20)

add_library(lua STATIC ${LUA_SOURCES})

target_include_directories(lua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
