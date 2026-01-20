#include <filesystem>
#include <print>
#include <string_view>
#include <vector>
#include <ranges>
#include <algorithm>
#include <cctype>

#include "coro-runtime.hxx"
#include "file-reader.hxx"
#include "coro-task.hxx"

namespace fs = std::filesystem;
namespace r = std::ranges;
namespace io = ribomation::io;

class Count {
    fs::path filename;
    unsigned lines = 0;
    unsigned words = 0;
    unsigned chars = 0;

public:
    explicit Count(fs::path const& filename_) : filename{filename_} {}

    void update(std::string const& line) {
        ++lines;
        words += words_of(line);
        chars += chars_of(line);
    }

    auto get_filename() const noexcept -> fs::path const& { return filename; }

    static auto header() -> std::string {
        return std::format("{:25} {:5} {:>7} {:>8}", "filename", "lines", "words", "chars");
    }

    auto to_string() const -> std::string {
        return std::format("{:25} {:5} {:7} {:8}",
                           filename.filename().string(), lines, words, chars);
    }

private:
    static auto words_of(std::string const& line) -> unsigned {
        auto is_letter = [&line](auto k) {
            return isalpha(static_cast<unsigned char>(line[k]));
        };

        auto result = 0U;
        auto k = std::size_t{0};
        do {
            for (; k < line.size() && not is_letter(k); ++k) {}
            if (k == line.size()) return result;
            ++result;
            for (; k < line.size() && is_letter(k); ++k) {}
        } while (k < line.size());

        return result;
    }

    static auto chars_of(std::string const& line) -> unsigned {
        return static_cast<unsigned>(line.size());
    }
};

class Results {
    std::vector<Count> results{};
public:
    void push(Count cnt) {
        results.push_back(std::move(cnt));
    }
    auto get() -> std::vector<Count> {
        r::sort(results, [](auto& lhs, auto& rhs) {
            return lhs.get_filename().string() < rhs.get_filename().string();
        });
        return std::move(results);
    }
};

auto CountFile(fs::path filename, Results& res, io::CoroRuntime& rt) -> io::TaskCoroutine<void> {
    auto reader = io::AsyncFileReader{filename, rt};
    auto cnt = Count{filename};
    while (auto line = co_await reader.next_line()) cnt.update(*line);
    res.push(std::move(cnt));
    co_return;
}

int main(int argc, char* argv[]) {
    auto dir = argc > 1 ? fs::path{argv[1]} : fs::path{"../../data"};
    if (not fs::is_directory(dir)) throw std::invalid_argument{"not a directory: " + dir.string()};

    auto runtime = io::CoroRuntime{};
    auto results = Results{};

    auto is_text_file = [](fs::path const& p) {
        return p.extension() == ".txt";
    };

    for (auto e : fs::directory_iterator{dir}) {
        if (not e.is_regular_file()) continue;
        if (not is_text_file(e.path())) continue;

        runtime.spawn( CountFile(e.path(), results, runtime) );
    }

    runtime.run();

    std::println("#) {}", Count::header());
    auto fileno = 1U;
    for (auto const& r : results.get()) {
        std::println("{}) {}", fileno++, r.to_string());
    }
}
