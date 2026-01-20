#pragma once
#include <coroutine>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace ribomation::io {

    class CoroScheduler {
        friend class CoroRuntime;
        mutable std::mutex  entry{};
        std::condition_variable not_empty{};
        std::queue<std::coroutine_handle<>> readyq{};

    public:
        void post(std::coroutine_handle<> coro) {
            {
                auto _ = std::lock_guard{entry};
                readyq.push(coro);
            }
            not_empty.notify_one();
        }

    private:
        bool is_empty() const { return readyq.empty(); }
    };

}


