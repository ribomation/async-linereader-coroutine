#include <iostream>
#include <print>
#include <string>
#include <filesystem>
#include <thread>
#include "file-reader.hxx"

namespace fs = std::filesystem;
namespace io = ribomation::io;

auto linesOf(fs::path file) -> unsigned {
    auto reader = io::FileReader{file};
    auto count = 0U;
    while (auto line = reader.next_line()) {
        ++count;
    }
    return count;
}

int main(int argc, char* argv[]) {
    auto dir = argc > 1 ? fs::path{argv[1]} : fs::path{"../../data"};
    if (not fs::is_directory(dir)) throw std::invalid_argument{"not a directory: " + dir.string()};

    for (auto e : fs::directory_iterator{dir}) {
        if (not e.is_regular_file()) continue;

        std::println("{}: {} lines ({} bytes)",
            e.path().filename().string(),
            linesOf(e.path()),
            e.file_size());
    }
}
