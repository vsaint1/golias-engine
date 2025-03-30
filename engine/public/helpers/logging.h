#pragma once

#include "imports.h"


#define STRINGIFY(x)   #x
#define TO_STRING(x)   STRINGIFY(x)
#define TRACE_FILE_LOG "[EMBER_ENGINE - " __FILE__ ":" TO_STRING(__LINE__) "] "

/*!

   @brief ERROR logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_ERROR(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, TRACE_FILE_LOG __VA_ARGS__);

/*!

   @brief INFO logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_INFO(...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, TRACE_FILE_LOG __VA_ARGS__);

/*!

   @brief DEBUG logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_DEBUG(...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, TRACE_FILE_LOG __VA_ARGS__);

/*!

   @brief VERBOSE logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_VERBOSE(...) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, TRACE_FILE_LOG __VA_ARGS__);

/*!

   @brief WARNING logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_WARN(...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, TRACE_FILE_LOG __VA_ARGS__);

/*!

   @brief TRACING logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_TRACE(...) SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION, TRACE_FILE_LOG __VA_ARGS__);

/*!

   @brief CRITICAL logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_CRITICAL(...) SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, TRACE_FILE_LOG __VA_ARGS__);

/*!

   @brief If fail the app will quit
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
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
   
*/
#define EMBER_TIMER_START() auto start = std::chrono::high_resolution_clock::now();


/*!

   @brief Timer macro end
   @version 0.0.4
   @returns the messsage with the duration in ms
*/
#define EMBER_TIMER_END(description)                                                             \
    do {                                                                                            \
        auto end = std::chrono::high_resolution_clock::now();                                        \
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(); \
        LOG_INFO("%s took %lld (µs), %.2f (ms)", description, duration, (float) duration / 1000.f);                             \
    } while (0)

