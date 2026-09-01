cmake_minimum_required(VERSION 3.18)

project(thirdparty)


# GLM
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_ENABLE_CXX_20 ON)
set(GLM_TEST_ENABLE OFF)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if(EMSCRIPTEN)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build STATIC libs" FORCE)

add_subdirectory(glad)
add_subdirectory(stb_image)
add_subdirectory(glfw EXCLUDE_FROM_ALL)
add_subdirectory(glm)

add_subdirectory(json)
add_subdirectory(tinygltfloader)
add_subdirectory(tinyobjloader)


set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF)
add_subdirectory(bullet3 EXCLUDE_FROM_ALL)

add_subdirectory(mini_audio)

add_subdirectory(stb_truetype)

add_library(thirdparty INTERFACE)

target_include_directories(${PROJECT_NAME}  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} INTERFACE  ${CMAKE_CURRENT_SOURCE_DIR}/bullet3/src)

target_link_libraries(${PROJECT_NAME}  INTERFACE
        stb_image
        glfw
        glm
        glad
        json
        tinygltfloader
        tinyobjloader
        BulletDynamics
        BulletSoftBody
        BulletCollision
        LinearMath
        mini_audio
        stb_truetype
)