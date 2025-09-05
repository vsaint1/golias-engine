cmake_minimum_required(VERSION 3.22)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ember_engine)

message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

# [CONFIGURATION]
#set(APP_NAME "Ember")
#set(APP_VERSION "1.0")
#set(APP_IDENTIFIER "com.emberengine.app.project")
## [ORIENTATION] - "LandscapeLeft" - "LandscapeRight" - "Portrait" - "PortraitUpsideDown"
#set(APP_ORIENTATION "LandscapeLeft")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

set(ASSETS_FOLDER ${CMAKE_SOURCE_DIR}/res)

if(NOT EXISTS ${ASSETS_FOLDER})
    message(STATUS "Copy the default resources folder to ${ASSETS_FOLDER}")
    message(FATAL_ERROR "Resources folder not found: ${ASSETS_FOLDER}")
endif()

include_directories(PUBLIC "${CMAKE_SOURCE_DIR}/engine/public")

if(WITH_EDITOR)
    add_subdirectory(vendor/imgui)
    add_compile_definitions(WITH_EDITOR)
endif ()

# [iOS/Emscripten] - disable shared libs
if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    set(SDL_SHARED OFF)
else()
    set(SDL_SHARED ON)
endif()

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)
    endif()
endif()


# [SOURCE_FILES] - add all game source files
file(GLOB_RECURSE SOURCES "runtime/*.cpp" "runtime/*.c")


add_subdirectory(vendor/glad)

add_subdirectory(vendor/stb_image)

add_subdirectory(vendor/mini_audio)

add_subdirectory(vendor/freetype)


# [SDL] - removing unused modules
set(SDL_RENDER OFF CACHE BOOL "" FORCE)
set(SDL_RENDER_GPU OFF CACHE BOOL "" FORCE)
set(SDL_DIRECTX OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)

add_subdirectory(vendor/tinyxml)


set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF)
set(GLM_ENABLE_CXX_20 ON)
set(GLM_TEST_ENABLE OFF)
add_subdirectory(vendor/glm)


set(CURL_USE_MBEDTLS ON CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)

set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/mbedtls)

set(MBEDX509_LIBRARY mbedx509)
set(MBEDCRYPTO_LIBRARY mbedcrypto)
set(MBEDTLS_LIBRARY mbedtls)

set(MBEDTLS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/mbedtls/include)
set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/curl EXCLUDE_FROM_ALL)

add_subdirectory(vendor/json)

add_subdirectory(vendor/box2d)

set_target_properties(box2d PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_subdirectory(engine)


# [iOS/macOS/Android/Linux] - commons
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing")
endif()

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")
    set(METAL_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/metal/default.metal)
endif()

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()


if(ANDROID)

    set(CMAKE_ANDROID_PACKAGE_NAME ${APP_IDENTIFIER})
    set(CMAKE_ANDROID_VERSION_NAME ${APP_VERSION})
    set(CMAKE_ANDROID_VERSION_CODE 1)

    file(GLOB_RECURSE ANDROID_FILES "engine/private/core/platform/android/*.c")

    # configure_file("${CMAKE_SOURCE_DIR}/templates/config/AndroidManifest.xml.in" "${CMAKE_BINARY_DIR}/templates/android/app/src/main/AndroidManifest.xml" @ONLY)

    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${ENGINE_SOURCES} ${ANDROID_FILES})
    target_link_libraries(${PROJECT_NAME} PUBLIC android log GLESv3 m dl)

endif()

# iOS [simulator/device]
if(APPLE AND NOT BUILD_SHARED_LIBS)
    set(LAUNCH_SCREEN "${CMAKE_SOURCE_DIR}/templates/ios/LaunchScreen.storyboard")
    set(ICON_PATH "${CMAKE_SOURCE_DIR}/templates/ios/AppLogo.icns")


    set(ASSETS_PATH "${CMAKE_SOURCE_DIR}/res")

    file(GLOB_RECURSE IOS_FILES "engine/private/core/platform/ios/*.mm")

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE  ${SOURCES} ${ENGINE_SOURCES} ${METAL_SHADERS} ${LAUNCH_SCREEN} ${ASSETS_PATH} ${IOS_FILES})

    find_library(IO ImageIO REQUIRED)
    find_library(CS CoreServices REQUIRED)
    find_library(CT CoreText REQUIRED)
    find_library(CG CoreGraphics REQUIRED)
    find_library(CF CoreFoundation REQUIRED)
    find_library(GL OpenGLES REQUIRED)
    find_library(CA CoreAudio REQUIRED)
    find_library(AU AudioToolBox REQUIRED)
    find_library(UKIT UIKit)

    target_link_libraries(${PROJECT_NAME} PUBLIC ${CS} ${CT} ${IO} ${CS} ${CG} ${GL} ${UKIT} ${CA} ${AU})

    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE_ICON_FILE ${ICON_PATH}
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/templates/ios/Info.plist.in"
            XCODE_IPHONEOS_DEPLOYMENT_TARGET 12.0
            XCODE_GENERATE_SCHEME TRUE
            XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES YES
            XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
            # XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER ${APP_IDENTIFIER}
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APP_IDENTIFIER}
            XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
    )

    # Ensure res are copied into the bundle
    set_source_files_properties(${ASSETS_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    install(DIRECTORY ${ASSETS_PATH} DESTINATION "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app/Contents/Resources")

    if(CMAKE_OSX_SYSROOT MATCHES "iphonesimulator")
        message(STATUS "SIMULATOR BUILD")
        set_target_properties(${PROJECT_NAME} PROPERTIES
                XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO"
                XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphonesimulator"
                XCODE_ATTRIBUTE_ARCHS "x86_64 arm64")
    else()
        message(STATUS "REAL DEVICE BUILD")
        set_target_properties(${PROJECT_NAME} PROPERTIES
                XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO"
                XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphoneos"
                XCODE_ATTRIBUTE_ARCHS "arm64")
    endif()
endif()


# macosx [arm]
if(APPLE AND BUILD_SHARED_LIBS)

    message(STATUS "BUILD MACOS")
    set(RESOURCES "${CMAKE_SOURCE_DIR}/res")
    file(GLOB_RECURSE MAC_FILES "engine/private/core/platform/macos/*.mm")
    file(GLOB_RECURSE MAC_FILES "engine/private/core/renderer/metal/*.mm")

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${ENGINE_SOURCES} ${METAL_SHADERS} ${RESOURCES} ${MAC_FILES} )

    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            RESOURCE "${RESOURCES}")

    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreGraphics" "-framework CoreAudio" "-framework AudioToolbox" "-framework OpenGL" "-framework QuartzCore" "-framework AppKit" "-framework Metal" "-framework IOKit")
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/engine/public)


endif()

# Windows [x86_x64/arm]
if(MSVC)

#   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    file(GLOB_RECURSE WIN_FILES "engine/private/core/platform/windows/*.c")

    add_executable(${PROJECT_NAME} ${SOURCES} ${ENGINE_SOURCES} ${WIN_FILES})

    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/res ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res)

    target_link_libraries(${PROJECT_NAME} PUBLIC opengl32 glu32)

    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)

endif()

# WASM [emscripten]
if(EMSCRIPTEN)
    set(PROJECT_NAME "index")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    file(GLOB_RECURSE WEB_FILES "engine/private/core/platform/web/*.c")

    add_executable(${PROJECT_NAME} ${SOURCES} ${ENGINE_SOURCES} ${WEB_FILES})

    set(SHELL_HTML_FILE "${CMAKE_SOURCE_DIR}/templates/web/shell.html")

    if(NOT EXISTS ${SHELL_HTML_FILE})
        message(FATAL_ERROR "Shell file not found: ${SHELL_HTML_FILE}")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file ${SHELL_HTML_FILE} -s USE_WEBGL2=1 -s FETCH=1 -s  FULL_ES3=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s STACK_SIZE=10MB -s ALLOW_MEMORY_GROWTH=1 --preload-file ${CMAKE_SOURCE_DIR}/res@/res")


endif()

# Linux [x86_x64/arm]
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    file(GLOB_RECURSE LINUX_FILES "engine/private/core/platform/linux/*.c")


    add_executable(${PROJECT_NAME} ${SOURCES} ${ENGINE_SOURCES} ${LINUX_FILES})

    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)

    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/res ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res)

endif()

# WARNING: HERE WE LINK ALL DEPENDENCIES, `MUST BE CROSS-PLATFORM AND NOT PLATFORM SPECIFIC`
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_link_libraries(${PROJECT_NAME} PUBLIC engine)

# target_compile_definitions(${PROJECT_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)
