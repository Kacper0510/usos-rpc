#include "logging.hpp"

#include <cstdlib>
#include <filesystem>
#include <utility>

#include "exceptions.hpp"

// Forward declaration
namespace usos_rpc {
    const std::filesystem::path* const get_config_directory();
}

namespace {

    /// @brief Regular expression for stripping ASCII color codes.
    const std::regex ASCII_COLOR_CODES(R"(\x1b\[[0-9;]*m)");

    /// @brief File to log all console output to, or a null pointer.
    std::unique_ptr<std::ofstream> log_file = nullptr;

}

bool usos_rpc::should_show_colored_output() {
    static const char* no_color = std::getenv("NO_COLOR");
    return no_color == nullptr || no_color[0] == '\0';
}

void usos_rpc::initialize_logging() {
    if (log_file != nullptr) {
        return;
    }

    auto path = *get_config_directory() / "service.log";
    auto stream = std::ofstream(path, std::ios::binary);
    if (!stream) {
        throw Exception(ExceptionType::IO, "Failed to initialize log file ({})!", path.string());
    }
    log_file = std::make_unique<std::ofstream>(std::move(stream));
}

void usos_rpc::log(std::ostream& stream, const std::string& to_print) {
    const auto stripped = std::regex_replace(to_print, ASCII_COLOR_CODES, "");

    if (usos_rpc::should_show_colored_output()) {
        stream << to_print;
    } else {
        stream << stripped;
    }

    if (log_file != nullptr) {
        *log_file << stripped;
    }
}
