cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(golias_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

# =========================================================
# OUTPUT DIRECTORIES
# TODO: separate the server and client output directories
# =========================================================
set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(ENGINE_VERSION "0.0.1")

string(TIMESTAMP BUILD_TS "%Y-%m-%d %H:%M")
message("Golias Engine - Building in 2D/3D mode | (OPENGL/VULKAN/METAL/DIRECTX12) | Version ${ENGINE_VERSION} | Date: ${BUILD_TS}")

# =========================================================
# ENGINE CONFIGURATION
# TODO: build client shared and server static
# =========================================================
option(BUILD_SANDBOX "Build Sandbox (engine development)" ON)
option(BUILD_GOLIAS_TESTS "Build tests" OFF)

message(STATUS "Build configuration:")
message(STATUS "  BUILD SANDBOX (ENGINE DEVELOPMENT): ${BUILD_SANDBOX}")
message(STATUS "  BUILD TESTS: ${BUILD_GOLIAS_TESTS}")
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
if (BUILD_GOLIAS_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (BUILD_SANDBOX)
    message(STATUS "Adding sandbox build")
    add_compile_definitions(BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif ()
