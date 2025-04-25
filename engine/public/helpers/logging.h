#pragma once

#include "imports.h"

/*
   @brief Class for logging, tracing and debugging

   - Web - Output to javascript console
   - Windows - Output to console and file
   - macOS - Output to xcode console
   - Linux - Output to console and file
   - Android - Output to logcat
   - iOS - Output to xcode console

   @version 0.0.9

*/
class Debug {
public:
    static Debug& Get() {
        static Debug instance;
        return instance;
    }

    static void Start() {
        auto& debug = Get();
        
        if(!debug.log_thread){
            return;
        }

        debug.running    = true;
        debug.log_thread = SDL_CreateThread(
            [](void* data) -> int {
                static_cast<Debug*>(data)->LogThread();
                return 0;
            },
            "LogThread", &debug);
    }

    void Log(const std::string& msg, bool save_to_disk = true) {
        std::lock_guard<std::mutex> lock(mutex);
        log_queue.push_back(msg);
        cond.notify_all();
    }

    static void Destroy() {
        auto& debug = Get();

        debug.mutex.lock();

        debug.running = false;

        debug.mutex.unlock();

        debug.cond.notify_all();

        SDL_WaitThread(debug.log_thread, nullptr);
    }

private:
    Debug()  = default;
    ~Debug() = default;


    void LogThread() {

        // TODO: Get from config file
        char* pref_path = SDL_GetPrefPath("ember", "com.emberengine.app");

        std::string log_path = std::string(pref_path).append("/ember_logs.txt");

        SDL_IOStream* file = SDL_IOFromFile(log_path.c_str(), "a");

        if (!file) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open log file: %s", SDL_GetError());
            return;
        }

        std::unique_lock<std::mutex> lock(mutex);

        while (running || !log_queue.empty()) {
            cond.wait(lock, [this]() { return !log_queue.empty() || !running; });

            while (!log_queue.empty()) {
                std::string msg = log_queue.front();
                log_queue.pop_front();

                lock.unlock();

                std::string full_msg = msg.append("\n");
                SDL_WriteIO(file, full_msg.c_str(), full_msg.size());

                lock.lock();
            }

            SDL_FlushIO(file);
        }

        SDL_CloseIO(file);
    }

    std::mutex mutex;
    SDL_Thread* log_thread = nullptr;
    std::condition_variable cond;
    std::atomic<bool> running = false;

    std::deque<std::string> log_queue;
};

#define STRINGIFY(x)   #x
#define TO_STRING(x)   STRINGIFY(x)
#define TRACE_FILE_LOG "[EMBER_ENGINE - " __FILE__ ":" TO_STRING(__LINE__) "] "

/*!

   @brief ERROR logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_ERROR(...)                                                    \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);         \
        Debug::Get().Log(buffer);                                         \
    } while (0)

/*!

   @brief INFO logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_INFO(...)                                                     \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);          \
        Debug::Get().Log(buffer);                                         \
    } while (0)

/*!

   @brief DEBUG logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_DEBUG(...)                                                    \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);         \
        Debug::Get().Log(buffer);                                         \
    } while (0)

/*!

   @brief VERBOSE logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_VERBOSE(...)                                                  \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);       \
        Debug::Get().Log(buffer);                                         \
    } while (0)

/*!

   @brief WARNING logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_WARN(...)                                                     \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);          \
        Debug::Get().Log(buffer);                                         \
    } while (0)

/*!

   @brief TRACING logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_TRACE(...)                                                    \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);          \
        Debug::Get().Log(buffer);                                         \
    } while (0)

/*!

   @brief CRITICAL logging
   @version 0.0.1
   @param string c-string with `printf` format specifier
   @returns just print the message
*/
#define LOG_CRITICAL(...)                                                 \
    do {                                                                  \
        char buffer[1024];                                                \
        SDL_snprintf(buffer, sizeof(buffer), TRACE_FILE_LOG __VA_ARGS__); \
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", buffer);      \
        Debug::Get().Log(buffer);                                         \
    } while (0)

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
#define EMBER_TIMER_END(description)                                                                \
    do {                                                                                            \
        auto end      = std::chrono::high_resolution_clock::now();                                  \
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(); \
        LOG_INFO("%s took %lld (us), %.2f (ms)", description, duration, (float) duration / 1000.f); \
    } while (0)
