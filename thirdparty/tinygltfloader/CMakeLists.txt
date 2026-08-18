cmake_minimum_required(VERSION 3.18)

project(tinygltfloader)

set(CMAKE_CXX_STANDARD 20)

add_library(tinygltfloader STATIC src/tiny_gltf.cpp)

target_include_directories(tinygltfloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tinygltfloader PUBLIC json stb_image)
