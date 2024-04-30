#pragma once

#include <algorithm>

#include "../files.hpp"
#include "../logging.hpp"
#include "build_info.hpp"

namespace usos_rpc::commands {

    /// @brief Prints the version of the program.
    void version() {
        usos_rpc::print(
            "USOS Discord Rich Presence {}\n{}\nhttps://{}\n\nDependencies:\n", FULL_VERSION, COPYRIGHT, GITHUB_URL
        );
        std::for_each(DEPENDENCIES.begin(), DEPENDENCIES.end(), [](const auto& dep) {
            print(" - {}\n", dep);
        });
    }

    /// @brief Prints the help message.
    void help() {
        usos_rpc::print(fmt::fg(fmt::color::orange), "TODO\n");
    }

    /// @brief Prints the auto-selected config directory.
    void config() {
        usos_rpc::print("{}\n", get_config_directory()->string());
    }

}
