#pragma once
#include <optional>
#include <coroutine>
#include <exception>
#include <utility>

namespace ribomation::io {

    //forward decl
    template<typename ValueType> struct TaskCoroutine;

    namespace impl {
        template<typename ValueType>
        struct TaskPromise {
            std::optional<ValueType>  value{};
            std::exception_ptr        error{};
            std::coroutine_handle<>   continuation{};

            struct CompletionAwaiter {
                bool await_ready() noexcept { return false; }
                void await_resume() noexcept {}
                void await_suspend(std::coroutine_handle<TaskPromise> h) noexcept {
                    if (auto cont = h.promise().continuation; cont) cont.resume();
                }
            };

            auto get_return_object() -> TaskCoroutine<ValueType>;
            auto initial_suspend() noexcept { return std::suspend_always{}; }
            auto final_suspend() noexcept { return CompletionAwaiter{}; }

            template<typename ReturnType>
            void return_value(ReturnType&& v) {
                value.emplace( std::forward<ReturnType>(v) );
            }
            void unhandled_exception() noexcept { error = std::current_exception(); }
        };

        template<>
        struct TaskPromise<void> {
            std::exception_ptr        error{};
            std::coroutine_handle<>   continuation{};

            struct CompletionAwaiter {
                bool await_ready() noexcept { return false; }
                void await_resume() noexcept {}
                void await_suspend(std::coroutine_handle<TaskPromise> h) noexcept {
                    if (auto cont = h.promise().continuation; cont) cont.resume();
                }
            };

            auto get_return_object() -> TaskCoroutine<void>;
            auto initial_suspend() noexcept { return std::suspend_always{}; }
            auto final_suspend() noexcept { return CompletionAwaiter{}; }

            void return_void() noexcept {}
            void unhandled_exception() noexcept { error = std::current_exception(); }
        };
    }


    template<typename ValueType>
    struct TaskCoroutine {
        using promise_type = impl::TaskPromise<ValueType>;
        using handle_type  = std::coroutine_handle<promise_type>;
    private:
        handle_type handle{};
    public:
        explicit TaskCoroutine(handle_type h) : handle(h) {}
        ~TaskCoroutine() { if (handle) handle.destroy(); }

        TaskCoroutine(TaskCoroutine const&) = delete;
        TaskCoroutine(TaskCoroutine && rhs) noexcept :handle(std::exchange(rhs.handle, {})) {}

        auto operator =(TaskCoroutine const&) -> TaskCoroutine& = delete;
        auto operator =(TaskCoroutine && rhs) noexcept -> TaskCoroutine& {
            if (this != &rhs) {
                if (handle) handle.destroy();
                handle = std::exchange(rhs.handle, {});
            }
            return *this;
        }

        void start() {
            if (handle && not handle.done()) handle.resume();
        }
        auto get_handle() const noexcept -> handle_type {
            return handle;
        }

        class LaunchAwaiter {
            handle_type this_task;
        public:
            explicit LaunchAwaiter(handle_type h) : this_task(h) {}

            bool await_ready() noexcept { return not this_task || this_task.done(); }
            void await_suspend(std::coroutine_handle<> other_task) noexcept {
                this_task.promise().continuation = other_task;
                this_task.resume();
            }
            auto await_resume() -> ValueType {
                if (auto& p = this_task.promise(); p.error) {
                    std::rethrow_exception(p.error);
                }
                return std::move( this_task.promise().value.value() );
            }
        };

        auto operator co_await() const noexcept -> LaunchAwaiter { return LaunchAwaiter{handle}; }
    };

    template<>
    struct TaskCoroutine<void> {
        using promise_type = impl::TaskPromise<void>;
        using handle_type  = std::coroutine_handle<promise_type>;
    private:
        handle_type handle{};
    public:
        explicit TaskCoroutine(handle_type h) : handle(h) {}
        ~TaskCoroutine() { if (handle) handle.destroy(); }

        TaskCoroutine(TaskCoroutine const&) = delete;
        TaskCoroutine(TaskCoroutine && rhs) noexcept :handle(std::exchange(rhs.handle, {})) {}

        auto operator =(TaskCoroutine const&) -> TaskCoroutine& = delete;
        auto operator =(TaskCoroutine && rhs) noexcept -> TaskCoroutine& {
            if (this != &rhs) {
                if (handle) handle.destroy();
                handle = std::exchange(rhs.handle, {});
            }
            return *this;
        }

        void start() {
            if (handle && not handle.done()) handle.resume();
        }
        auto get_handle() const noexcept -> handle_type {
            return handle;
        }

        class LaunchAwaiter {
            handle_type this_task;
        public:
            explicit LaunchAwaiter(handle_type h) : this_task(h) {}
            bool await_ready() noexcept { return not this_task || this_task.done(); }
            void await_suspend(std::coroutine_handle<> other_task) noexcept {
                this_task.promise().continuation = other_task;
                this_task.resume();
            }
            void await_resume() {
                if (auto& p = this_task.promise(); p.error) {
                    std::rethrow_exception(p.error);
                }
            }
        };

        auto operator co_await() const noexcept -> LaunchAwaiter { return LaunchAwaiter{handle}; }
    };

}

namespace ribomation::io::impl {
    template<typename ValueType>
    auto TaskPromise<ValueType>::get_return_object() -> TaskCoroutine<ValueType> {
        auto h = std::coroutine_handle<TaskPromise>::from_promise(*this);
        return TaskCoroutine<ValueType>{h};
    }

    inline auto TaskPromise<void>::get_return_object() -> TaskCoroutine<void> {
        auto h = std::coroutine_handle<TaskPromise>::from_promise(*this);
        return TaskCoroutine<void>{h};
    }
}
