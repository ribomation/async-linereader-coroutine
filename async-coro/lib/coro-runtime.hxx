#pragma once
#include <atomic>
#include <coroutine>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "coro-scheduler.hxx"
#include "task-pool.hxx"
#include "coro-task.hxx"


namespace ribomation::io {
    class CoroRuntime; //forward decl

    class LivenessToken {
        CoroRuntime* runtime = nullptr;

    public:
        LivenessToken() = default;
        explicit LivenessToken(CoroRuntime* runtime);
        ~LivenessToken();

        LivenessToken(LivenessToken const&) = delete;
        auto operator=(LivenessToken const&) -> LivenessToken& = delete;

        LivenessToken(LivenessToken&& rhs) noexcept
            : runtime{std::exchange(rhs.runtime, nullptr)} {}

        auto operator=(LivenessToken&& rhs) noexcept -> LivenessToken& {
            if (this != &rhs) {
                this->~LivenessToken();
                runtime = std::exchange(rhs.runtime, nullptr);
            }
            return *this;
        }
    };

    struct SpawnedEntry {
        LivenessToken token;
        TaskCoroutine<void> task;
    };

    class CoroRuntime {
        friend class LivenessToken;
        using SpawnedKeyType = void*;

        CoroScheduler sched{};
        TaskPool pool{};
        std::atomic<std::size_t> active_tasks{0};

        std::unordered_map<SpawnedKeyType, SpawnedEntry> spawned{};
        std::mutex entry{};

        bool has_more_work() const {
            return not sched.is_empty() || active_tasks.load() == 0U;
        }

        bool no_more_work() const {
            return active_tasks.load() == 0U && sched.is_empty();
        }

    public:
        explicit CoroRuntime(unsigned num_workers = std::thread::hardware_concurrency())
            : pool{num_workers} {}

        auto scheduler() -> CoroScheduler& { return sched; }
        auto task_pool() -> TaskPool& { return pool; }
        auto make_token() -> LivenessToken { return LivenessToken{this}; }

        void spawn(TaskCoroutine<void>&& task);
        void run();

    private:
        void acquire_liveness() {
            active_tasks.fetch_add(1U, std::memory_order_relaxed);
        }

        void release_liveness() {
            active_tasks.fetch_sub(1U, std::memory_order_relaxed);
            sched.not_empty.notify_all();
        }
    };

    inline LivenessToken::LivenessToken(CoroRuntime* runtime_) : runtime{runtime_} {
        runtime->acquire_liveness();
    }

    inline LivenessToken::~LivenessToken() {
        if (runtime != nullptr) runtime->release_liveness();
    }
}
