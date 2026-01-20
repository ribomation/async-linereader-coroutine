#include "task-pool.hxx"

namespace ribomation::io {
    void TaskPool::worker_loop() {
        while (true) {
            auto task = std::move_only_function<void()>{};

            {
                auto guard = std::unique_lock{entry};
                not_empty.wait(guard, [this] {
                    return shutting_down || not tasks.empty();
                });
                if (shutting_down && tasks.empty()) return;

                task = std::move(tasks.front());
                tasks.pop();
            }

            task(); //run the task
        }
    }
}
