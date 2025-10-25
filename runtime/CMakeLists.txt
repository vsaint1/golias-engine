cmake_minimum_required(VERSION 3.18)

project(golias_runtime)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)


# =========================================================
#  ASSETS CONFIGURATION
# =========================================================
set(ASSETS_FOLDER "${CMAKE_SOURCE_DIR}/res")

if (NOT EXISTS ${ASSETS_FOLDER})
    message(STATUS "Copy the default resources folder to ${ASSETS_FOLDER}")
    message(FATAL_ERROR "Resources folder not found: ${ASSETS_FOLDER}")
endif ()

message(STATUS "Using assets folder: ${ASSETS_FOLDER}")


# =========================================================
#  PLATFORM SPECIFIC CONFIGURATION
# =========================================================
if (UNIX AND NOT EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing -frtti -fexceptions -fPIC -Wall -pthread")
endif ()

if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")
endif ()

if (MSVC)
    if (NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)
    endif ()
endif ()

if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif ()

# =========================================================
#  SOURCE FILES
# =========================================================
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.c")


# =========================================================
#  PLATFORM-SPECIFIC EXECUTABLES (RUNTIME)
# =========================================================

# -----------------------------
# ANDROID
# -----------------------------
if (ANDROID)

    add_library(${PROJECT_NAME} SHARED ${SOURCES})

    set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME "client"
            SUFFIX ".so")

    target_link_libraries(${PROJECT_NAME} PUBLIC android log GLESv3 m dl)
endif ()

# -----------------------------
# iOS (Device/Simulator)
# -----------------------------
if (APPLE AND NOT BUILD_SHARED_LIBS)
    set(LAUNCH_SCREEN "${CMAKE_SOURCE_DIR}/templates/ios/LaunchScreen.storyboard")
    set(ICON_PATH "${CMAKE_SOURCE_DIR}/templates/ios/AppLogo.icns")
    set(ASSETS_PATH "${CMAKE_SOURCE_DIR}/res")

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE
            ${SOURCES} ${LAUNCH_SCREEN} ${ASSETS_PATH})

    find_library(IO ImageIO REQUIRED)
    find_library(CS CoreServices REQUIRED)
    find_library(CT CoreText REQUIRED)
    find_library(CG CoreGraphics REQUIRED)
    find_library(CF CoreFoundation REQUIRED)
    find_library(GL OpenGLES REQUIRED)
    find_library(CA CoreAudio REQUIRED)
    find_library(AU AudioToolBox REQUIRED)
    find_library(UK UIKit REQUIRED)

    target_link_libraries(${PROJECT_NAME} PUBLIC ${CS} ${CT} ${IO} ${CG} ${GL} ${UK} ${CA} ${AU})

    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE_ICON_FILE ${ICON_PATH}
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/templates/ios/Info.plist.in"
            XCODE_IPHONEOS_DEPLOYMENT_TARGET 12.0
            XCODE_GENERATE_SCHEME TRUE
            XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES YES
            XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APP_IDENTIFIER}
            XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}")

    set_source_files_properties(${ASSETS_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    install(DIRECTORY ${ASSETS_PATH} DESTINATION "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app/Contents/Resources")

    if (CMAKE_OSX_SYSROOT MATCHES "iphonesimulator")
        message(STATUS "SIMULATOR BUILD")
        set_target_properties(${PROJECT_NAME} PROPERTIES
                XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO"
                XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphonesimulator"
                XCODE_ATTRIBUTE_ARCHS "x86_64 arm64")
    else ()
        message(STATUS "REAL DEVICE BUILD")
        set_target_properties(${PROJECT_NAME} PROPERTIES
                XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO"
                XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphoneos"
                XCODE_ATTRIBUTE_ARCHS "arm64")
    endif ()
endif ()

# -----------------------------
# macOS (x86_64/ARM64)
# -----------------------------
if (APPLE AND BUILD_SHARED_LIBS)
    set(RESOURCES "${CMAKE_SOURCE_DIR}/res")

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE
            ${SOURCES} ${RESOURCES})

    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            RESOURCE "${RESOURCES}")

    target_link_libraries(${PROJECT_NAME} PRIVATE
            "-framework CoreGraphics"
            "-framework CoreAudio"
            "-framework AudioToolbox"
            "-framework OpenGL"
            "-framework QuartzCore"
            "-framework AppKit"
            "-framework Metal"
            "-framework IOKit")

    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/engine/public)
endif ()

# -----------------------------
# Windows (MSVC)
# TODO: MinGW?
# -----------------------------
if (WIN32)

    add_executable(${PROJECT_NAME} ${SOURCES})

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/res ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res)

    target_link_libraries(${PROJECT_NAME} PUBLIC opengl32 glu32)

    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif ()

# -----------------------------
# WASM (Emscripten)
# -----------------------------
if (EMSCRIPTEN)
    add_executable(${PROJECT_NAME} ${SOURCES})

    set(EM_FLAGS
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sNO_DISABLE_EXCEPTION_CATCHING=1"
        "-sDISABLE_EXCEPTION_THROWING=0"
        "-sDISABLE_EXCEPTION_CATCHING=0"
        "-sFETCH=1"
        "-sASYNCIFY"
        "-sFORCE_FILESYSTEM=1"
        "-sSTACK_SIZE=10MB"
        "-sINITIAL_MEMORY=128MB"
        "--preload-file ${CMAKE_SOURCE_DIR}/res@/res"
    )

    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-c++11-narrowing)

    string(REPLACE ";" " " EM_FLAGS_STR "${EM_FLAGS}")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
        COMPILE_FLAGS "${EM_FLAGS_STR}"
        LINK_FLAGS "${EM_FLAGS_STR}"  
    )

    # Optional: custom shell (HTML template)
    # set(SHELL_HTML_FILE "${CMAKE_SOURCE_DIR}/templates/web/shell.html")
    # if (EXISTS ${SHELL_HTML_FILE})
    #     set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
    #         "${EM_FLAGS_STR} --shell-file ${SHELL_HTML_FILE}")
    # endif ()
endif()


# =========================================================
#  COMMON TARGET SETTINGS
# =========================================================
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PUBLIC engine)