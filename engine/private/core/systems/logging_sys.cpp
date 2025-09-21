#include "core/systems/logging_sys.h"


Logger& Logger::get_instance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const char* app_identifier) {
    auto& debug = get_instance();

    if (debug._thread) {
        return;
    }

    auto fn_thread = [](void* data) -> int {
        static_cast<Logger*>(data)->log_thread();
        return 0;
    };

    debug._app_identifier = app_identifier;
    debug._is_running     = true;
    debug._thread         = SDL_CreateThread(fn_thread, "LogThread", &debug);
}

void Logger::push(const std::string& formatted_log) {

    if (!_is_running || _thread == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(_mutex);
    _log_queue.push_back(formatted_log);
    _condition.notify_all();
}

void Logger::destroy() {
    auto& debug = get_instance();

    debug._mutex.lock();

    debug._is_running = false;

    debug._mutex.unlock();

    debug._condition.notify_all();

    int status = 0;
    SDL_WaitThread(debug._thread, &status);
}


void Logger::log_thread() {

    char* pref_path = SDL_GetPrefPath("Ember Engine", _app_identifier);

    time_t now = time(nullptr);
    tm local_time = {};

#if defined(SDL_PLATFORM_WINDOWS)
    errno_t err = localtime_s(&local_time, &now);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get local time: %d", err);
        return;
    }
#else
    if (localtime_r(&now, &local_time) == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get local time");
        return;
    }
#endif

    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d-%S", &local_time);

    char file_name[256];
    SDL_snprintf(file_name, sizeof(file_name), "engine_logs-%s.txt", buffer);

    const std::string log_path = std::string(pref_path).append(file_name);

    SDL_IOStream* file = SDL_IOFromFile(log_path.c_str(), "a");

    if (!file) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open log file: %s", SDL_GetError());
        return;
    }

    std::unique_lock lock(_mutex);

    while (_is_running || !_log_queue.empty()) {
        _condition.wait(lock, [this]() { return !_log_queue.empty() || !_is_running; });

        while (!_log_queue.empty()) {
            std::string msg = _log_queue.front();
            _log_queue.pop_front();

            lock.unlock();

            std::string full_msg = msg.append("\n");
            SDL_WriteIO(file, full_msg.c_str(), full_msg.size());

            lock.lock();
        }

        SDL_FlushIO(file);
    }

    SDL_CloseIO(file);
}
