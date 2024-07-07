/// @file
/// @brief Informational commands' definitions.

#pragma once

#include <algorithm>

#include "../files.hpp"
#include "../logging.hpp"
#include "build_info.hpp"

#include "battery/embed.hpp"

namespace usos_rpc::commands {

    /// @brief Prints the version of the program.
    void version() {
        lprint("USOS Discord Rich Presence {}\n{}\nhttps://{}\n\nDependencies:\n", FULL_VERSION, COPYRIGHT, GITHUB_URL);
        std::for_each(DEPENDENCIES.begin(), DEPENDENCIES.end(), [](const auto& dep) {
            lprint(" - {}\n", dep);
        });
    }

    /// @brief Prints the help message.
    void help() {  // clang-format off
        #ifdef _WIN32
            auto windows_help = b::embed<"resources/windows.ansi">().data();
        #else
            constexpr const char* windows_help = "";
        #endif
        lprint(
            fmt::runtime(b::embed<"resources/help.ansi">().data()),
            fmt::arg("version", VERSION),
            fmt::arg("exe_name", get_executable_path().filename().string()),
            fmt::arg("github_url", GITHUB_URL),
            fmt::arg("windows", windows_help)
        );
    }  // clang-format on

    /// @brief Prints the auto-selected config directory.
    void config() {
        lprint("{}\n", get_config_directory()->string());
    }

}
