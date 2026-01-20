#pragma once
#include <coroutine>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include "coro-runtime.hxx"

namespace ribomation::io {
    namespace fs = std::filesystem;

    class AsyncFileReader {
        std::ifstream file{};
        CoroRuntime& runtime;

    public:
        AsyncFileReader(fs::path filename, CoroRuntime& runtime_)
            : file{std::move(filename)}, runtime{runtime_} {
            if (not file) throw std::invalid_argument{"cannot open " + filename.string()};
        }

        struct NextLineAwaitable {
            struct State {
                std::optional<std::string> line{};
                std::exception_ptr error{};
                bool eof = false;
            };

            std::shared_ptr<State> state = std::make_shared<State>();

            std::ifstream& file;
            CoroRuntime& runtime;

        public:
            NextLineAwaitable(std::ifstream& file_, CoroRuntime& runtime_) : file{file_}, runtime{runtime_} {}
            bool await_ready() noexcept { return false; }
            void await_suspend(std::coroutine_handle<> invoking_coro);
            auto await_resume() -> std::optional<std::string>;
        };

        auto next_line() -> NextLineAwaitable {
            return NextLineAwaitable{file, runtime};
        }
    };
}
