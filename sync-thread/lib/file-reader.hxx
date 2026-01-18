#pragma once
#include <fstream>
#include <string>
#include <filesystem>
#include <optional>

namespace ribomation::io {
    namespace fs = std::filesystem;

    /**
     * A simple file reader that reads text files line by line.
     * 
     * This class provides a straightforward interface for reading files sequentially,
     * one line at a time. The file is opened in the constructor and automatically
     * closed when the object is destroyed.
     * 
     * @throws std::invalid_argument if the file cannot be opened
     */
    class FileReader {
        std::ifstream file;
    public:
        explicit FileReader(fs::path const& filename) : file{filename} {
            if (not file) throw std::invalid_argument{"cannot open " + filename.string()};
        }

        /**
         * Reads the next line from the file.
         * 
         * This method reads and returns the next line from the file. If a line is
         * successfully read, it is returned wrapped in a std::optional. When the end
         * of the file is reached or an error occurs during reading, std::nullopt is
         * returned.
         * 
         * @return std::optional<std::string> containing the next line if available,
         *         or std::nullopt if the end of file is reached or an error occurs
         */
        auto next_line()  -> std::optional<std::string>;
    };

    /**
     * Removes all non-alphabetic characters from the input text.
     * 
     * This function filters the input string, keeping only alphabetic characters
     * (as determined by std::isalpha) and discarding all other characters including
     * digits, whitespace, and punctuation. The resulting string contains only
     * letters from the input text in their original order.
     * 
     * @param text The input string to be cleaned
     * @return std::string A new string containing only the alphabetic characters
     *         from the input text
     */
    auto clean(std::string const& text) -> std::string;
}
