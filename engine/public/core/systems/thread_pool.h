#pragma once
#include "engine_sys.h"
#include "helpers/logging.h"

class ThreadManager final: public EngineManager {
public:
    explicit ThreadManager(size_t threads);
    ~ThreadManager() override;

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

    bool initialize() override;

    void update(double delta_time) override;

    void shutdown() override;

private:
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> tasks;

    std::mutex _queue_mutex;
    std::condition_variable condition;
    bool is_running = false;
};


template <class F, class... Args>
auto ThreadManager::enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
#if defined(SDL_PLATFORM_EMSCRIPTEN)
    LOG_INFO("ThreadPool not supported on Web builds.");
#else

    using return_type = decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();

    {
        std::unique_lock lock(_queue_mutex);
        if (!is_running) {
            throw std::runtime_error("ThreadPool isn't running, cannot enqueue new tasks.");
        }

        tasks.emplace([task]() { (*task)(); });
    }

    condition.notify_one();
    return res;
#endif

}
