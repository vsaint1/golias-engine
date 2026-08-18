#pragma once

#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#define GLM_FORCE_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <json/json.hpp>


#ifdef _WIN32

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h>

#endif


#ifdef NDEBUG

    #define GOLIAS_ASSERT(x) ((void) 0)

#else

    #define GOLIAS_ASSERT(x)                                  \
        do {                                                  \
            if (!(x)) {                                       \
                GOLIAS_LOG_ERROR("Assertion failed: {}", #x); \
                assert(x);                                    \
            }                                                 \
        } while (0)


    #define GOLIAS_ASSERT_MSG(x, msg)                          \
        do {                                                   \
            if (!(x)) {                                        \
                GOLIAS_LOG_ERROR("Assertion failed: {}", msg); \
                assert(x);                                     \
            }                                                  \
        } while (0)


#endif


#define GOLIAS_LOG_TRACE(fmt, ...) std::printf("[Trace] %s - " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

#define GOLIAS_LOG_INFO(fmt, ...) std::printf("[Info] %s - " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

#define GOLIAS_LOG_WARN(fmt, ...) std::printf("[Warn] %s - " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

#define GOLIAS_LOG_DEBUG(fmt, ...) std::printf("[Debug] %s - " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

#define GOLIAS_LOG_ERROR(fmt, ...) std::printf("[Error] %s - " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

#define GOLIAS_LOG_CRITICAL(fmt, ...) std::printf("[Critical] %s - " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

#define GOLIAS_LOG_FATAL(fmt, ...)                                          \
    do {                                                                    \
        std::printf("[Fatal] %s - " fmt "\n", __FUNCTION__, ##__VA_ARGS__); \
        std::abort();                                                       \
    } while (false)


// OS helper

#if defined(_WIN32) || defined(_WIN64)

    #define GOLIAS_PLATFORM_WINDOWS

#elif defined(__APPLE__)

    #define GOLIAS_PLATFORM_APPLE 1

    #include <TargetConditionals.h>

    #if TARGET_OS_OSX
        #define GOLIAS_PLATFORM_OSX 1
    #elif TARGET_OS_IOS
        #define GOLIAS_PLATFORM_IOS 1
    #elif TARGET_OS_TV
        #define GOLIAS_PLATFORM_TVOS 1
    #elif TARGET_OS_WATCH
        #define GOLIAS_PLATFORM_WATCHOS 1
    #endif


#elif defined(__ANDROID__)
    #define GOLIAS_PLATFORM_ANDROID 1

#elif defined(__linux__)
    #define GOLIAS_PLATFORM_LINUX 1

#elif defined(__EMSCRIPTEN__)
    #define GOLIAS_PLATFORM_EMSCRIPTEN 1

#else
    #error "Unsupported platform"

#endif

namespace golias {


    template <class T>
    using Ref = std::shared_ptr<T>;

    template <class T>
    using Scope = std::unique_ptr<T>;

    template <class T>
    using WeakRef = std::weak_ptr<T>;

    using String = std::string;

    using Json = nlohmann::json;

} // namespace golias
