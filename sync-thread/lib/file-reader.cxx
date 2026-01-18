#include <cctype>
#include "file-reader.hxx"

namespace ribomation::io {

    auto FileReader::next_line()  -> std::optional<std::string> {
        auto line = std::string{};
        line.reserve(256);
        std::getline(file, line);
        if (file.good()) return line;
        return std::nullopt;
    }

    auto clean(std::string const& text) -> std::string {
        auto result = std::string{};
        result.reserve(text.size());
        for (char const ch : text) {
            if (std::isalpha(ch)) result += ch;
        }
        return result;
    }
}
