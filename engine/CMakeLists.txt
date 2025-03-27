cmake_minimum_required(VERSION 3.18)

project(engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

# ENGINE SOURCE
file(GLOB_RECURSE ENGINE_SOURCES "private/*.cpp", "private/*.cpp" "private/*.c")

add_library(engine STATIC ${ENGINE_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3 glad stb_image stb_truetype glm::glm mini_audio)
