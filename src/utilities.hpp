#pragma once

#include <algorithm>
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
    [[nodiscard]]
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

    /// @brief Removes whitespace from the beginning and the end of a string.
    /// @param input string to process
    /// @return stripped/trimmed string
    [[nodiscard]]
    std::string strip(const std::string& input) {
        std::size_t start = 0;
        std::size_t end = input.size();
        while (start < end && std::isspace(input[start])) {
            start++;
        }
        while (end > start && std::isspace(input[end - 1])) {
            end--;
        }
        return input.substr(start, end - start);
    }

    /// @brief Checks if a container contains a value.
    /// @tparam T container type
    /// @tparam V value type
    /// @param container container to check
    /// @param value value to match
    /// @return true if the value is in the container
    template <typename T, typename V>
    [[nodiscard]]
    bool contains(const T& container, const V& value) {
        return std::find(container.begin(), container.end(), value) != container.end();
    }

    /// @brief Creates a standard library array from a list of values without specifying size.
    /// Copied from https://gist.github.com/klmr/2775736.
    /// @tparam ...T types of values
    /// @param ...values values to put in the array
    /// @return std::array of values
    template <typename... T>
    constexpr std::array<typename std::decay<typename std::common_type<T...>::type>::type, sizeof...(T)> make_array(
        T&&... values
    ) {
        return { std::forward<T>(values)... };
    }
}
