/// @file
/// @brief Functions for logging and colored output.

#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>

#include "fmt/color.h"
#include "fmt/format.h"

namespace usos_rpc {

    /// @brief Returns true if the output of this program should be colored.
    /// @return true if NO_COLOR environment variable is not set
    [[nodiscard]]
    bool should_show_colored_output();

    /// @brief Initializes log file (service.log).
    /// @throws usos_rpc::Exception when file stream creation fails
    void initialize_logging();

    /// @brief Prints string to the selected stream and optionally writes it to the log file.
    /// @param stream selected output stream
    /// @param to_print string to print
    void log(std::ostream& stream, const std::string& to_print);

    /// @brief Prints formatted input to stdout and optionally writes it to the log file.
    /// @tparam ...T types to interpolate
    /// @param fmt_str format string with curly braces
    /// @param ...args arguments to interpolate
    template <typename... T>
    void lprint(fmt::format_string<T...> fmt_str, T&&... args) {
        auto to_print = fmt::vformat(fmt_str, fmt::make_format_args(args...));
        log(std::cout, to_print);
    }

    /// @brief Prints formatted input to stdout and optionally writes it to the log file.
    /// @tparam ...T types to interpolate
    /// @param ts text style
    /// @param fmt_str format string with curly braces
    /// @param ...args arguments to interpolate
    template <typename... T>
    void lprint(const fmt::text_style& ts, fmt::format_string<T...> fmt_str, T&&... args) {
        auto to_print = fmt::vformat(ts, fmt_str, fmt::make_format_args(args...));
        log(std::cout, to_print);
    }

    /// @brief Prints formatted input to stderr and optionally writes it to the log file.
    /// @tparam ...T types to interpolate
    /// @param fmt_str format string with curly braces
    /// @param ...args arguments to interpolate
    template <typename... T>
    void eprint(fmt::format_string<T...> fmt_str, T&&... args) {
        auto to_print = fmt::vformat(fmt_str, fmt::make_format_args(args...));
        log(std::clog, to_print);
    }

    /// @brief Prints formatted input to stderr and optionally writes it to the log file.
    /// @tparam ...T types to interpolate
    /// @param ts text style
    /// @param fmt_str format string with curly braces
    /// @param ...args arguments to interpolate
    template <typename... T>
    void eprint(const fmt::text_style& ts, fmt::format_string<T...> fmt_str, T&&... args) {
        auto to_print = fmt::vformat(ts, fmt_str, fmt::make_format_args(args...));
        log(std::clog, to_print);
    }

    namespace colors {
        /// @brief Exception warnings formatting style.
        constexpr fmt::text_style WARNING = fmt::fg(fmt::terminal_color::bright_yellow);

        /// @brief Fatal errors formatting style.
        constexpr fmt::text_style FATAL_ERROR = fmt::fg(fmt::terminal_color::bright_red);

        /// @brief Formatting style for messages indicating some kind of a success.
        constexpr fmt::text_style SUCCESS = fmt::fg(fmt::terminal_color::bright_green);

        /// @brief Formatting style for other messages that should stand out.
        constexpr fmt::text_style OTHER = fmt::fg(fmt::terminal_color::bright_cyan);
    }

}
