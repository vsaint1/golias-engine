#include  "core/systems/thread_pool.h"


ThreadPool::ThreadPool(size_t threads)
    : is_running(true) // pool starts running
{
    for (size_t i = 0; i < threads; ++i) {
        _workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;

                {
                    std::unique_lock lock(this->_queue_mutex);
                    this->condition.wait(lock, [this] { return !this->is_running || !this->tasks.empty(); });

                    if (!this->is_running && this->tasks.empty())
                        return;

                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::shutdown() {
    {
        std::unique_lock lock(_queue_mutex);
        is_running = false;
    }
    condition.notify_all();

    for (auto& worker : _workers)
        if (worker.joinable())
            worker.join();
}
