#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace usos_rpc {

    /// @brief calculates C-string's length at compile time
    /// @tparam N length that will be returned
    /// @param string
    /// @return N - 1
    template <std::size_t N>
    constexpr std::size_t const_string_length(const char (&)[N]) {
        return N - 1;
    }

    /// @brief Splits a string into a vector of strings by a delimiter.
    /// @param input string to split
    /// @param delimiter where to split
    /// @return vector of strings
    std::vector<std::string> split(const std::string& input, const std::string& delimiter) {
        std::vector<std::string> result;
        std::size_t start = 0;
        std::size_t end = input.find(delimiter);
        while (end != std::string::npos) {
            result.push_back(input.substr(start, end - start));
            start = end + delimiter.size();
            end = input.find(delimiter, start);
        }
        result.push_back(input.substr(start, end));
        return result;
    }

}
