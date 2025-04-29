#include "helpers/logging.h"

void Debug::Start() {
    auto& debug = Get();

    if (debug.log_thread) {
        return;
    }

    auto fn_thread = [](void* data) -> int {
        static_cast<Debug*>(data)->LogThread();
        return 0;
    };

    debug.running    = true;
    debug.log_thread = SDL_CreateThread(fn_thread, "LogThread", &debug);
}

void Debug::Push(const std::string& formatted_log) {

    std::lock_guard<std::mutex> lock(mutex);
    log_queue.push_back(formatted_log);
    cond.notify_all();
}

void Debug::Destroy() {
    auto& debug = Get();

    debug.mutex.lock();

    debug.running = false;

    debug.mutex.unlock();

    debug.cond.notify_all();

    int status = 0;
    SDL_WaitThread(debug.log_thread, &status);

}


void Debug::LogThread() {

    // TODO: Get from config file
    char* pref_path = SDL_GetPrefPath("Ember", "com.emberengine.app");

    time_t now = time(nullptr);
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
