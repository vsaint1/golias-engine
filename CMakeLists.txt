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

set(ENGINE_VERSION "1.0.0")

add_definitions(-DENGINE_VERSION_STR="${ENGINE_VERSION}")

message("Ember Engine - Building in 2D/3D mode | (OPENGL/VULKAN/METAL/DIRECTX12) | Version ${ENGINE_VERSION} | Date: ${CMAKE_BUILD_DATE}")
# =========================================================
# ENGINE CONFIGURATION
# TODO: build client shared and server static
# =========================================================
option(WITH_EDITOR "Build the engine with editor support" OFF)
option(BUILD_SERVER "Build server binaries" OFF)
option(BUILD_RUNTIME "Build runtime binaries" ON)
option(BUILD_EMBER_TESTS "Build tests" OFF)

message(STATUS "Build configuration:")
message(STATUS "  WITH_EDITOR: ${WITH_EDITOR}")
message(STATUS "  BUILD RUNTIME (CLIENT): ${BUILD_RUNTIME}")
message(STATUS "  BUILD RUNTIME (SERVER): ${BUILD_SERVER}")
message(STATUS "  BUILD TESTS: ${BUILD_EMBER_TESTS}")
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
# VENDOR LIBRARIES AND CONFIGURATION
# =========================================================

add_subdirectory(vendor)

# =========================================================
# ENGINE PROJECT
# =========================================================
add_subdirectory(engine)


# =========================================================
# TESTS
# =========================================================
if (BUILD_EMBER_TESTS)
    enable_testing()
endif ()

if (BUILD_EMBER_TESTS)
    add_subdirectory(tests)
endif ()

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

