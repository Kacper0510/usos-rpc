/// @file
/// @brief Functions for logging and colored output.

#pragma once

#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <type_traits>

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

    enum class TextStyle {
        /// @brief Exception warnings formatting style.
        WARNING,
        /// @brief Fatal errors formatting style.
        FATAL_ERROR,
        /// @brief Formatting style for messages indicating some kind of a success.
        SUCCESS,
        /// @brief Formatting style for other messages that should stand out.
        OTHER,
        /// @brief Internal style to reset formatting.
        RESET,
    };

    constexpr std::string to_ansi_escape(TextStyle style) noexcept {
        using enum TextStyle;
        switch (style) {
            case WARNING:
                return "\033[93m"; // bright yellow
            case FATAL_ERROR:
                return "\033[91m"; // bright red
            case SUCCESS:
                return "\033[92m"; // bright green
            case OTHER:
                return "\033[96m"; // bright cyan
            default:
                return "\033[0m"; // reset
        }
    }

    template <typename T>
    constexpr std::string styled(const T& value, TextStyle style) {
        return std::format("{}{}", to_ansi_escape(style), value);
    }

    template <typename T>
    struct ResetStyle {
        T&& value;
        TextStyle style;
    };

    template <typename T>
    ResetStyle(T&&, TextStyle) -> ResetStyle<T>;

    /// @brief Prints formatted input to stdout and optionally writes it to the log file.
    /// @tparam ...T types to interpolate
    /// @param fmt_str format string with curly braces
    /// @param ...args arguments to interpolate
    template <typename... T>
    void lprint(std::format_string<ResetStyle<std::type_identity_t<T>>...> fmt_str, T&&... args) {
        lprint(TextStyle::RESET, fmt_str, std::forward<T>(args)...);
    }

    /// @brief Prints formatted input to stdout and optionally writes it to the log file.
    /// @tparam ...T types to interpolate
    /// @param ts text style
    /// @param fmt_str format string with curly braces
    /// @param ...args arguments to interpolate
    template <typename... T>
    void lprint(TextStyle ts, std::format_string<ResetStyle<std::type_identity_t<T>>...> fmt_str, T&&... args) {
        auto to_print = std::format(fmt_str, ResetStyle{std::forward<T>(args), ts}...);
        log(std::cout, to_ansi_escape(ts) + to_print + to_ansi_escape(TextStyle::RESET));
    }

    /// @brief Prints formatted input to stderr and optionally writes it to the log file.
    /// @tparam ...T types to interpolate
    /// @param fmt_str format string with curly braces
    /// @param ...args arguments to interpolate
    template <typename... T>
    void eprint(std::format_string<ResetStyle<std::type_identity_t<T>>...> fmt_str, T&&... args) {
        eprint(TextStyle::RESET, fmt_str, std::forward<T>(args)...);
    }

    /// @brief Prints formatted input to stderr and optionally writes it to the log file.
    /// @tparam ...T types to interpolate
    /// @param ts text style
    /// @param fmt_str format string with curly braces
    /// @param ...args arguments to interpolate
    template <typename... T>
    void eprint(TextStyle ts, std::format_string<ResetStyle<std::type_identity_t<T>>...> fmt_str, T&&... args) {
        auto to_print = std::format(fmt_str, ResetStyle{std::forward<T>(args), ts}...);
        log(std::clog, to_ansi_escape(ts) + to_print + to_ansi_escape(TextStyle::RESET));
    }

}

namespace std {

    template <typename T>
    struct formatter<usos_rpc::ResetStyle<T>> {
        using Inner = remove_cvref_t<T>;
        formatter<Inner> inner_formatter;

        constexpr auto parse(format_parse_context& ctx) {
            return inner_formatter.parse(ctx);
        }

        auto format(const usos_rpc::ResetStyle<T>& wrapper, format_context& ctx) const {
            inner_formatter.format(wrapper.value, ctx);
            return format_to(ctx.out(), "{}", usos_rpc::to_ansi_escape(wrapper.style));
        }
    };

}
