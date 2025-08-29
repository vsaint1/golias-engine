cmake_minimum_required(VERSION 3.18)

project(engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

file(GLOB_RECURSE ENGINE_SOURCES "private/*.cpp" "private/*.c")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "engine/private/core/platform/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "engine/private/core/renderer/.mm")

add_library(engine STATIC ${ENGINE_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3 glad stb_image glm::glm mini_audio tinyxml json imgui freetype CURL::libcurl MbedTLS::mbedtls MbedTLS::mbedx509 MbedTLS::mbedcrypto box2d)

