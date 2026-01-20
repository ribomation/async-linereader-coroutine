#include "file-reader.hxx"

namespace ribomation::io {
    void AsyncFileReader::NextLineAwaitable::await_suspend(std::coroutine_handle<> invoking_coro) {
        auto token = runtime.make_token();

        auto task = [
                s = state,
                &input = file,
                &sched = runtime.scheduler(),
                invoking_coro,
                token = std::move(token)
            ] mutable {
            try {
                if (auto line = std::string{}; std::getline(input, line)) {
                    s->line = std::move(line);
                } else {
                    s->eof = true;
                }
            } catch (...) {
                s->error = std::current_exception();
            }

            sched.post(invoking_coro);
        };

        runtime.task_pool().submit(std::move(task));
    }

    auto AsyncFileReader::NextLineAwaitable::await_resume() -> std::optional<std::string> {
        if (state->error) std::rethrow_exception(state->error);
        if (state->eof) return std::nullopt;
        return std::move(state->line);
    }
}
