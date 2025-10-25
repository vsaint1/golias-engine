cmake_minimum_required(VERSION 3.18)

project(vendor)


set(SDL_GPU OFF CACHE BOOL "" FORCE)
set(SDL_RENDER_GPU OFF CACHE BOOL "" FORCE)
set(SDL_DIRECTX OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

# GLM
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_ENABLE_CXX_20 ON)
set(GLM_TEST_ENABLE OFF)

#SDL_image
set(SDLIMAGE_VENDORED OFF CACHE BOOL "Use the vendored libraries" FORCE)

# ASSIMP
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build ASSIMP tests" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "Build ASSIMP with internal ZLIB" FORCE)

# FLECS
set(FLECS_SHARED OFF CACHE BOOL "Build SHARED flecs lib" FORCE)

#SDL_ttf
set(SDLTTF_VENDORED ON CACHE BOOL "Use the vendored libraries" FORCE)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build STATIC libs" FORCE)

add_subdirectory(glad)
add_subdirectory(stb_image)
add_subdirectory(mini_audio)
add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(tinyxml)
add_subdirectory(glm)
add_subdirectory(json)
add_subdirectory(SDL_ttf)
add_subdirectory(lua)
add_subdirectory(sol2)
add_subdirectory(assimp)
add_subdirectory(flecs)
add_subdirectory(spdlog)

add_library(vendor INTERFACE)

target_include_directories(vendor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} INTERFACE
    glad
    stb_image
    mini_audio
    SDL3::SDL3
    SDL3_ttf-static
    tinyxml
    glm
    json
    lua
    sol2
    assimp
    flecs
    spdlog
)