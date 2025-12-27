cmake_minimum_required(VERSION 3.18)

project(thirdparty)


set(SDL_EXAMPLES_LINK_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)


# GLM
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_ENABLE_CXX_20 ON)
set(GLM_TEST_ENABLE OFF)


set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build STATIC libs" FORCE)

add_subdirectory(glad)
add_subdirectory(stb_image)
add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(glm)
add_subdirectory(spdlog)

add_subdirectory(json)
add_subdirectory(cgltf)

add_library(thirdparty INTERFACE)

target_include_directories(thirdparty INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}  INTERFACE
        stb_image
        SDL3::SDL3
        glm
        glad
        spdlog
        json
        cgltf
)