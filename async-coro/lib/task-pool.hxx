#pragma once
#include <vector>
#include <queue>
#include <functional>
#include <utility>
#include <thread>
#include <mutex>
#include <condition_variable>


namespace ribomation::io {

    class TaskPool {
        std::mutex entry{};
        std::condition_variable not_empty{};
        std::queue<std::move_only_function<void()>> tasks{};
        std::vector<std::thread> workers{};
        bool shutting_down = false;

    public:
        explicit TaskPool(unsigned num_workers = std::thread::hardware_concurrency()) {
            if (num_workers == 0U) num_workers = 1U;
            workers.reserve(num_workers);
            for (auto k = 0U; k < num_workers; ++k)
                workers.emplace_back([this] { worker_loop(); });
        }

        ~TaskPool() {
            {
                auto _ = std::lock_guard{entry};
                shutting_down = true;
            }
            not_empty.notify_all();
            for (auto& w : workers) w.join();
        }

        template<typename TaskType>
        void submit(TaskType&& task) {
            {
                auto _ = std::lock_guard{entry};
                tasks.emplace(std::forward<TaskType>(task));
            }
            not_empty.notify_one();
        }

    private:
        void worker_loop();
    };

}
