cmake_minimum_required(VERSION 3.22)

project(ember_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

# =========================================================
# OUTPUT DIRECTORIES
# TODO: separate the server and client output directories
# =========================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =========================================================
# ENGINE CONFIGURATION
# TODO: build client shared and server static
# =========================================================
option(WITH_EDITOR "Build the engine with editor support" OFF)
option(BUILD_SERVER "Build server binaries" ON)
option(BUILD_RUNTIME "Build runtime binaries" ON)

message(STATUS "Build configuration:")
message(STATUS "  WITH_EDITOR: ${WITH_EDITOR}")
message(STATUS "  BUILD_SERVER: ${BUILD_SERVER}")
message(STATUS "  BUILD_RUNTIME: ${BUILD_RUNTIME}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")

# =========================================================
# INCLUDE DIRECTORIES
# =========================================================
include_directories(PUBLIC "${CMAKE_SOURCE_DIR}/engine/public")


# =========================================================
#  PLATFORM LIBRARIES CONFIGURATION
#  - Windows/macOS/Linux - SHARED
#  - iOS/Wasm - STATIC
# =========================================================
if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(SDL_SHARED OFF)
else ()
    set(SDL_SHARED ON)
endif ()

# =========================================================
#  PLATFORM LIBRARIES CONFIGURATION
#  - Windows/macOS/Linux - SHARED
#  - iOS/Wasm - STATIC
# =========================================================
if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(SDL_SHARED OFF)
else()
    set(SDL_SHARED ON)
endif ()

# =========================================================
# OPTIONAL EDITOR
# =========================================================
if (WITH_EDITOR)
    add_subdirectory(vendor/imgui)
    add_compile_definitions(WITH_EDITOR)
endif ()

# =========================================================
# VENDOR LIBRARIES CONFIGURATION
# =========================================================

# SDL
set(SDL_GPU OFF CACHE BOOL "" FORCE)
set(SDL_RENDER_GPU OFF CACHE BOOL "" FORCE)
set(SDL_DIRECTX OFF CACHE BOOL "" FORCE)

# GLM
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_ENABLE_CXX_20 ON)
set(GLM_TEST_ENABLE OFF)

# CURL + MbedTLS
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CURL_USE_MBEDTLS ON CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

set(MBEDX509_LIBRARY mbedx509)
set(MBEDCRYPTO_LIBRARY mbedcrypto)
set(MBEDTLS_LIBRARY mbedtls)
set(MBEDTLS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/mbedtls/include")

# =========================================================
# ADD VENDOR SUBDIRECTORIES
# =========================================================
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build STATIC libs" FORCE)

add_subdirectory(vendor/glad)
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/mini_audio)
add_subdirectory(vendor/freetype)
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendor/tinyxml)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/enet-1.3.18)
add_subdirectory(vendor/json)
add_subdirectory(vendor/box2d)

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build SHARED libs" FORCE)
set(CURL_STATICLIB OFF CACHE BOOL "Build curl as shared" FORCE)

if (NOT EMSCRIPTEN)
    add_subdirectory(vendor/mbedtls)
    add_subdirectory(vendor/curl)
endif ()

# =========================================================
# ENGINE / SERVER / RUNTIME
# =========================================================
add_subdirectory(engine)

if (BUILD_SERVER)
    message(STATUS "Adding server build")
    add_compile_definitions(BUILD_SERVER)
    add_subdirectory(server)
endif ()

if (BUILD_RUNTIME)
    message(STATUS "Adding runtime build")
    add_compile_definitions(BUILD_RUNTIME)
    add_subdirectory(runtime)
endif ()
