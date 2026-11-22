/// @file
/// @brief Exception handling for usos-rpc.

#pragma once

#include <exception>
#include <string>
#include <string_view>

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
        /// @brief Indicates Discord RPC exception.
        DISCORD,
        /// @brief Indicates configuration file loading error.
        CONFIG,
        /// @brief Indicates Windows registry error.
        REGISTRY,
        /// @brief Indicates systemd command error.
        SYSTEMD,
    };

    /// @brief Exception type formatting function for std::format.
    /// @param type exception type
    /// @return string view of the formatted exception category
    constexpr std::string_view to_string_view(ExceptionType type) noexcept {
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
            case DISCORD:
                return "Discord";
            case CONFIG:
                return "wrong config";
            case REGISTRY:
                return "registry";
            case SYSTEMD:
                return "systemd command";
            default:
                return "unknown";
        }
    }

}

namespace std {

    template <>
    struct formatter<usos_rpc::ExceptionType> : formatter<string_view> {
        auto format(usos_rpc::ExceptionType type, format_context& ctx) const {
            return formatter<string_view>::format(usos_rpc::to_string_view(type), ctx);
        }
    };

}

namespace usos_rpc {

    /// @brief Exception class for usos-rpc.
    /// All constructors log thrown exception to stderr.
    class Exception: public std::exception {
        /// @brief Explains what happened.
        std::string _message;
        /// @brief Contains type of the exception.
        ExceptionType _type;

    public:
        /// @brief Constructor from a C-string.
        /// @param type exception type
        /// @param message exception message
        Exception(ExceptionType type, const char* message): _message(message), _type(type) {
            eprint(TextStyle::WARNING, "Warning - {}: {}\n", _type, _message);
        }

        /// @brief Constructor from an std::string.
        /// @param type exception type
        /// @param message exception message
        Exception(ExceptionType type, const std::string& message): _message(message), _type(type) {
            eprint(TextStyle::WARNING, "Warning - {}: {}\n", _type, _message);
        }

        /// @brief Constructor based on std::format.
        /// @tparam ...T types to interpolate
        /// @param type exception type
        /// @param fmt_str format string with curly braces
        /// @param ...args arguments to interpolate
        template <typename... T>
        Exception(ExceptionType type, std::format_string<T...> fmt_str, T&&... args) : _type(type),
            _message(std::format(fmt_str, std::forward<T>(args)...)) {
            eprint(TextStyle::WARNING, "Warning - {}: {}\n", _type, _message);
        }

        /// @brief Returns the exception message.
        /// @return exception message
        const char* what() const noexcept override {
            return _message.c_str();
        }

        virtual ~Exception() {}

        friend struct std::formatter<Exception>;
    };

}

namespace std {

    template <>
    struct formatter<usos_rpc::Exception> {
        constexpr auto parse(format_parse_context& ctx) {
            auto it = ctx.begin();
            if (it != ctx.end() && *it != '}') {
                throw format_error("Invalid format specifier for Exception");
            }
            return it;
        }

        auto format(const usos_rpc::Exception& e, format_context& ctx) const {
            return format_to(ctx.out(), "{}: {}", e._type, e._message);
        }
    };

}
