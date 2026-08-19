cmake_minimum_required(VERSION 3.18)
project(engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

add_definitions(-DENGINE_VERSION="${ENGINE_VERSION}")

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "private/*.cpp" "private/*.c")

if(APPLE)
    file(GLOB_RECURSE METAL_SOURCES CONFIGURE_DEPENDS "private/graphics/rhi/metal/*.mm")
    list(APPEND ENGINE_SOURCES ${METAL_SOURCES})
endif()

list(FILTER ENGINE_SOURCES EXCLUDE REGEX "engine/private/core/platform/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "engine/private/core/renderer/.mm")

if(APPLE)
    list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/graphics/rhi/vulkan/.*")
endif()


if(WIN32)
    file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS "engine/private/core/platform/windows/*.cpp" "engine/private/core/platform/windows/*.c")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS "engine/private/core/platform/linux/*.cpp" "engine/private/core/platform/linux/*.c")

elseif(ANDROID)
    file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS "engine/private/core/platform/android/*.cpp" "engine/private/core/platform/android/*.c")

elseif(APPLE)
    if (CMAKE_OSX_SYSROOT MATCHES ".*iPhoneOS.*" OR CMAKE_OSX_SYSROOT MATCHES ".*iPhoneSimulator.*")
        file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS
                "engine/private/core/platform/ios/*.mm"
                "engine/private/core/platform/ios/*.m"
        )
    else()
        file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS
                "engine/private/core/platform/macos/*.mm"
                "engine/private/core/platform/macos/*.m"
        )
    endif()

elseif(EMSCRIPTEN)
    file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS
            "engine/private/core/platform/emscripten/*.cpp"
            "engine/private/core/platform/emscripten/*.c"
    )

else()
    message(FATAL_ERROR "Unsupported platform")
endif()

list(APPEND ENGINE_SOURCES ${PLATFORM_SOURCES})

add_library(engine STATIC ${ENGINE_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC thirdparty)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif()


# PCXX
target_precompile_headers(${PROJECT_NAME} PRIVATE "public/stdafx.h")