cmake_minimum_required(VERSION 3.18)

project(engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

add_definitions(-DENGINE_VERSION="${ENGINE_VERSION}")

set(ENGINE_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/private")
set(PLATFORM_DIR "${ENGINE_PRIVATE_DIR}/core/platform")

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "${ENGINE_PRIVATE_DIR}/*.cpp" "${ENGINE_PRIVATE_DIR}/*.c")

# Platform

if(WIN32)
    set(PLATFORM_NAME "Windows")
elseif(ANDROID)
    set(PLATFORM_NAME "Android")
elseif(EMSCRIPTEN)
    set(PLATFORM_NAME "Emscripten")
elseif(APPLE)
    if(CMAKE_OSX_SYSROOT MATCHES ".*iPhoneOS.*" OR CMAKE_OSX_SYSROOT MATCHES ".*iPhoneSimulator.*")
        set(PLATFORM_NAME "iOS")
    else()
        set(PLATFORM_NAME "macOS")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_NAME "Linux")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(PLATFORM_IMPL_DIR "${PLATFORM_DIR}/${PLATFORM_NAME}")

message(STATUS "Engine platform: ${PLATFORM_NAME}")

# Platform sources
#
# platform/foo.cpp                 -> default implementation
# platform/windows/foo.cpp         -> Windows override

file(GLOB PLATFORM_DEFAULT_SOURCES CONFIGURE_DEPENDS "${PLATFORM_DIR}/*.cpp" "${PLATFORM_DIR}/*.c")

set(PLATFORM_SOURCES)

foreach(DEFAULT_SOURCE IN LISTS PLATFORM_DEFAULT_SOURCES)
    get_filename_component(FILENAME "${DEFAULT_SOURCE}" NAME)
    set(PLATFORM_SOURCE "${PLATFORM_IMPL_DIR}/${FILENAME}")

    if(EXISTS "${PLATFORM_SOURCE}")
        list(APPEND PLATFORM_SOURCES "${PLATFORM_SOURCE}")
        message(STATUS "Platform override: ${FILENAME} -> ${PLATFORM_NAME}/${FILENAME}")
    else()
        list(APPEND PLATFORM_SOURCES "${DEFAULT_SOURCE}")
        message(STATUS "Platform default: ${FILENAME}")
    endif()
endforeach()

# Remove all platform files from source list.
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/private/core/platform/[^/]+/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/private/core/platform/[^/]+\\.(cpp|c)$")

list(APPEND ENGINE_SOURCES ${PLATFORM_SOURCES})

# Engine

add_library(engine STATIC ${ENGINE_SOURCES})

target_link_libraries(engine PUBLIC thirdparty)

if(WIN32)
    target_link_libraries(engine PUBLIC psapi)
endif()

# Precompiled header

target_precompile_headers(engine PRIVATE "${ENGINE_PRIVATE_DIR}/../public/stdafx.h")