#pragma once

#include "stdafx.h"


#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)
#define TRACE_FILE_LOG \
    "[" __TIME__ "]"   \
    "[EMBER_ENGINE][" __FILE__ ":" TO_STRING(__LINE__) "]: "


    /*!
   @brief Class for logging, tracing and debugging

   - Web - Output to javascript console
   - Windows - Output to console and file
   - macOS - Output to xcode console
   - Linux - Output to console and file
   - Android - Output to logcat
   - iOS - Output to xcode console

   Use the LOG_ERROR, LOG_INFO, LOG_WARN, LOG_DEBUG macros to log messages

   @note Do not push sensitive data to the logs

   @ingroup Logging

   @version 0.0.9

*/
class Logger {
public:
    static Logger& get_instance();

    static void initialize(const char* app_identifier = "com.ember.engine.app");

    void push(std::string_view formatted_log);


private:
    Logger()  = default;
    ~Logger();

    void log_thread();

    const char* _app_identifier = "com.ember.engine.app";

    std::mutex _mutex                  = std::mutex();
    SDL_Thread* _thread                = nullptr;
    std::condition_variable _condition = std::condition_variable();
    std::atomic<bool> _is_running = false;

    std::deque<std::string_view> _log_queue = std::deque<std::string_view>();

    SDL_IOStream* file = nullptr;
};

/*!

   @brief ERROR logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
   @ingroup Logging
*/
#define LOG_ERROR(...)                                                    \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);         \
        Logger::get_instance().push(buffer);                              \
    } while (0)

/*!

   @brief INFO logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
   @ingroup Logging
*/
#define LOG_INFO(...)                                                     \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);          \
        Logger::get_instance().push(buffer);                              \
    } while (0)

/*!

   @brief DEBUG logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
   @ingroup Logging
*/
#define LOG_DEBUG(...)                                                    \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);         \
        Logger::get_instance().push(buffer);                              \
    } while (0)

/*!

   @brief VERBOSE logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
   @ingroup Logging
*/
#define LOG_VERBOSE(...)                                                  \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);       \
        Logger::get_instance().push(buffer);                              \
    } while (0)

/*!

   @brief WARNING logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
   @ingroup Logging
*/
#define LOG_WARN(...)                                                     \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);          \
        Logger::get_instance().push(buffer);                              \
    } while (0)

/*!

   @brief TRACING logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
   @ingroup Logging
*/
#define LOG_TRACE(...)                                                    \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);          \
        Logger::get_instance().push(buffer);                              \
    } while (0)

/*!

   @brief CRITICAL logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
   @ingroup Logging
*/
#define LOG_CRITICAL(...)                                                 \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);      \
        Logger::get_instance().push(buffer);                              \
    } while (0)

/*!

   @brief If fail the app will quit
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
   @ingroup Logging
*/
#define LOG_QUIT_ON_FAIL(x)              \
    if (!x) {                            \
        LOG_ERROR("%s", SDL_GetError()); \
        return SDL_APP_FAILURE;          \
    }


/*!

   @brief OpenGL error debug macro (for development)
   @version 0.0.1
   @returns just print the message
   @ingroup Logging
*/
#define GL_ERROR()                                  \
    {                                               \
        unsigned int error = glGetError();          \
        if (error != GL_NO_ERROR) {                 \
            LOG_ERROR("API ERROR_CODE: %d", error); \
        }                                           \
    }

/*!

   @brief Timer macro
   @version 0.0.4
   @ingroup Time
*/
#define EMBER_TIMER_START() auto start = std::chrono::high_resolution_clock::now();


/*!

   @brief Timer macro end
   @version 0.0.4
   @returns the messsage with the duration in ms
   @ingroup Time
*/
#define EMBER_TIMER_END(description)                                                                \
    do {                                                                                            \
        auto end      = std::chrono::high_resolution_clock::now();                                  \
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(); \
        LOG_INFO("%s took %lld (us), %.2f (ms)", description, duration, (float) duration / 1000.f); \
    } while (0)