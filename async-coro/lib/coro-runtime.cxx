#include "coro-runtime.hxx"

namespace ribomation::io {
    void CoroRuntime::spawn(TaskCoroutine<void>&& task) {
        auto h = task.get_handle();
        auto key = static_cast<SpawnedKeyType>(h.address());

        auto _ = std::lock_guard{entry};
        auto token = LivenessToken{this};

        auto [iter, inserted] = spawned.emplace(key, SpawnedEntry{std::move(token), std::move(task)});
        if (not inserted) return;

        iter->second.task.start();
        if (iter->second.task.get_handle().done()) spawned.erase(iter);
    }

    void CoroRuntime::run() {
        while (true) {
            auto handle = std::coroutine_handle<>{};

            {
                auto guard = std::unique_lock{sched.entry};
                sched.not_empty.wait(guard, [this] { return has_more_work(); });
                if (no_more_work()) return; //when active_tasks==0

                handle = sched.readyq.front();
                sched.readyq.pop();
            }

            handle.resume();
            if (handle.done()) {
                auto key = static_cast<SpawnedKeyType>(handle.address());
                auto _ = std::lock_guard{entry};
                spawned.erase(key);
            }
        }
    }
}
