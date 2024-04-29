#pragma once

#include <exception>
#include <string>

#include "logging.hpp"

namespace usos_rpc {
    /// @brief Represents the type of an exception.
    enum class ExceptionType {
        /// @brief Indicates filesystem error.
        IO_ERROR,
        /// @brief Indicates libcurl/networking error.
        CURL_ERROR,
        /// @brief Indicates parsing error.
        PARSE_ERROR,
    };

    /// @brief Exception type formatting function for fmt.
    /// @param type exception type
    /// @return formatted string
    inline auto format_as(const ExceptionType& type) {
        using enum ExceptionType;
        switch (type) {
            case IO_ERROR:
                return "filesystem error";
            case CURL_ERROR:
                return "networking error";
            case PARSE_ERROR:
                return "parsing error";
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
        Exception(ExceptionType type, const char* message): _message(message), _type(type) {
            eprint(WARNING_STYLE, "Warning - {}: {}\n", _type, _message);
        }

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

        const char* what() const noexcept override {
            return _message.c_str();
        }

        virtual ~Exception() {}

        /// @brief Exception formatting support for fmt.
        /// @param type exception type
        /// @return formatted string
        friend auto format_as(const Exception& e) {
            return fmt::format("{}: {}", e._type, e._message);
        }
    };
}
