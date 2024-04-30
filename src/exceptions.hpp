/// @file
/// @brief Exception handling for usos-rpc.

#pragma once

#include <exception>
#include <string>

#include "logging.hpp"

namespace usos_rpc {
    /// @brief Represents the type of an exception.
    enum class ExceptionType {
        /// @brief Indicates filesystem error.
        IO,
        /// @brief Indicates libcurl/networking error.
        CURL,
        /// @brief Indicates iCalendar parsing error.
        ICALENDAR,
        /// @brief Indicates command line argument parser error.
        ARGUMENTS,
    };

    /// @brief Exception type formatting function for fmt.
    /// @param type exception type
    /// @return formatted string
    inline auto format_as(const ExceptionType& type) {
        using enum ExceptionType;
        switch (type) {
            case IO:
                return "filesystem";
            case CURL:
                return "networking";
            case ICALENDAR:
                return "iCalendar parsing";
            case ARGUMENTS:
                return "wrong arguments";
            default:
                return "unknown error";
        }
    }

    /// @brief Exception class for usos-rpc.
    /// All constructors log thrown exception to stderr.
    class Exception: public std::exception {
        /// @brief Explains what happened.
        std::string _message;
        /// @brief Contains type of the exception.
        ExceptionType _type;

        /// @brief How exception warnings should be formatted.
        constexpr static fmt::text_style WARNING_STYLE = fmt::fg(fmt::terminal_color::yellow);

    public:
        /// @brief Constructor from a C-string.
        /// @param type exception type
        /// @param message exception message
        Exception(ExceptionType type, const char* message): _message(message), _type(type) {
            eprint(WARNING_STYLE, "Warning - {}: {}\n", _type, _message);
        }

        /// @brief Constructor from an std::string.
        /// @param type exception type
        /// @param message exception message
        Exception(ExceptionType type, const std::string& message): _message(message), _type(type) {
            eprint(WARNING_STYLE, "Warning - {}: {}\n", _type, _message);
        }

        /// @brief Constructor based on fmt::format.
        /// @tparam ...T types to interpolate
        /// @param fmt_str format string with curly braces
        /// @param ...args arguments to interpolate
        /// @param type exception type
        template <typename... T>
        Exception(ExceptionType type, fmt::format_string<T...> fmt_str, T&&... args): _type(type) {
            _message = fmt::vformat(fmt_str, fmt::make_format_args(args...));
            eprint(WARNING_STYLE, "Warning - {}: {}\n", _type, _message);
        }

        /// @brief Returns the exception message.
        /// @return exception message
        const char* what() const noexcept override {
            return _message.c_str();
        }

        virtual ~Exception() {}

        /// @brief Exception formatting support for fmt.
        /// @param e the exception to format
        /// @return formatted string
        friend auto format_as(const Exception& e) {
            return fmt::format("{}: {}", e._type, e._message);
        }
    };
}
