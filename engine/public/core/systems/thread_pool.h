#pragma once
#include "helpers/logging.h"

class ThreadPool {
public:
    explicit ThreadPool(size_t threads);
    ~ThreadPool();

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

    void shutdown();

private:
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> tasks;

    std::mutex _queue_mutex;
    std::condition_variable condition;
    bool is_running = false;
};


template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
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
}
