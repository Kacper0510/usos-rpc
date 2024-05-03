/// @file
/// @brief Informational commands' definitions.

#pragma once

#include <algorithm>

#include "../files.hpp"
#include "../logging.hpp"
#include "build_info.hpp"

namespace usos_rpc::commands {

    /// @brief Prints the version of the program.
    void version() {
        lprint("USOS Discord Rich Presence {}\n{}\nhttps://{}\n\nDependencies:\n", FULL_VERSION, COPYRIGHT, GITHUB_URL);
        std::for_each(DEPENDENCIES.begin(), DEPENDENCIES.end(), [](const auto& dep) {
            lprint(" - {}\n", dep);
        });
    }

    /// @brief Prints the help message.
    void help() {
        lprint(fmt::fg(fmt::color::orange), "TODO\n");
    }

    /// @brief Prints the auto-selected config directory.
    void config() {
        lprint("{}\n", get_config_directory()->string());
    }

}
