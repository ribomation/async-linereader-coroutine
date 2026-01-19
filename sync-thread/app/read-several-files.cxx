#include <cctype>
#include <print>
#include <string>
#include <filesystem>
#include <thread>
#include <future>
#include <vector>
#include "file-reader.hxx"

namespace fs = std::filesystem;
namespace io = ribomation::io;

class Count {
    fs::path filename{};
    unsigned lines{};
    unsigned words{};
    unsigned chars{};

public:
    explicit Count(fs::path filename_) : filename{std::move(filename_)} {}

    void count() {
        auto reader = io::FileReader{filename};
        while (auto line = reader.next_line()) {
            ++lines;
            words += wordsOf(*line);
            chars += charsOf(*line);
        }
    }

    auto to_string() const -> std::string {
        return std::format("{:25} {:5} {:7} {:8}",
                           filename.filename().string(), lines, words, chars);
    }

    static auto header() -> std::string {
        return std::format("{:25} {:5} {:>7} {:>8}", "filename", "lines", "words", "chars");
    }

private:
    auto charsOf(std::string const& line) -> unsigned {
        return static_cast<unsigned>(line.size());
    }

    auto wordsOf(std::string const& line) -> unsigned {
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
};

struct Task {
    fs::path filename{};
    std::promise<Count> result{};
    explicit Task(fs::path filename_) : filename{std::move(filename_)} {}

    void operator()() {
        auto cnt = Count{filename};
        cnt.count();
        result.set_value(cnt);
    }
};

int main(int argc, char* argv[]) {
    auto dir = argc > 1 ? fs::path{argv[1]} : fs::path{"../../data"};
    if (not fs::is_directory(dir)) throw std::invalid_argument{"not a directory: " + dir.string()};

    auto workers = std::vector<std::jthread>{};
    auto results = std::vector<std::future<Count>>{};

    for (auto e : fs::directory_iterator{dir}) {
        if (not e.is_regular_file()) continue;

        auto task = Task{e.path()};
        results.push_back(task.result.get_future());
        workers.emplace_back(std::jthread{std::move(task)});
    }

    auto fileno = 1U;
    std::println("#) {}", Count::header());
    for (auto& f : results) {
        auto cnt = f.get();
        std::println("{}) {}", fileno++, cnt.to_string());
    }
}
