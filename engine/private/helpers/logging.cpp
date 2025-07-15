#include "helpers/logging.h"


Logger& Logger::get_instance() {
    static Logger instance;
    return instance;
}

void Logger::initialize() {
    auto& debug = get_instance();

    if (debug._thread) {
        return;
    }

    auto fn_thread = [](void* data) -> int {
        static_cast<Logger*>(data)->_log_thread();
        return 0;
    };

    debug._bIsRunning = true;
    debug._thread     = SDL_CreateThread(fn_thread, "LogThread", &debug);
}

void Logger::push(const std::string& formatted_log) {

    std::lock_guard<std::mutex> lock(_mutex);
    _log_queue.push_back(formatted_log);
    _condition.notify_all();
}

void Logger::destroy() {
    auto& debug = get_instance();

    debug._mutex.lock();

    debug._bIsRunning = false;

    debug._mutex.unlock();

    debug._condition.notify_all();

    int status = 0;
    SDL_WaitThread(debug._thread, &status);
}


void Logger::_log_thread() {

    // TODO: Get from config file
    char* pref_path = SDL_GetPrefPath("Ember", "com.emberengine.app");

    time_t now   = time(nullptr);
    tm* timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d-%S", timeinfo);

    char file_name[256];
    SDL_snprintf(file_name, sizeof(file_name), "ember_logs.%s.txt", buffer);

    std::string log_path = std::string(pref_path).append(file_name);

    SDL_IOStream* file = SDL_IOFromFile(log_path.c_str(), "a");

    if (!file) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open log file: %s", SDL_GetError());
        return;
    }

    std::unique_lock lock(_mutex);

    while (_bIsRunning || !_log_queue.empty()) {
        _condition.wait(lock, [this]() { return !_log_queue.empty() || !_bIsRunning; });

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
