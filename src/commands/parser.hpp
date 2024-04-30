/// @file
/// @brief Command line arguments parsing helper.

#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "../utilities.hpp"

namespace usos_rpc::commands {

    /// @brief Creates a vector out of C-style command line arguments.
    /// @param argc number of command line arguments
    /// @param argv command line arguments
    /// @return vector of command line arguments in reverse order
    [[nodiscard]]
    std::vector<std::string> create_arguments_vector(const int argc, const char* argv[]) {
        std::vector<std::string> args(argv + 1, argv + argc);
        std::reverse(args.begin(), args.end());
        return args;
    }

    /// @brief Checks if the command line arguments are a specific command.
    /// Also accepts a short form and forms with hyphens.
    /// @param args command line arguments
    /// @param name command name
    /// @param alias optional command alias
    /// @return true if the command was detected, false otherwise
    bool check_command(
        std::vector<std::string>& args, const char* name, const std::optional<const char*> alias = std::nullopt
    ) {
        if (args.empty()) {
            return false;
        }

        auto to_check = args.back();
        if (to_check == name || to_check == std::string("--") + name) {
            args.pop_back();
            return true;
        }

        std::string short_name(1, name[0]);
        if (to_check == short_name || to_check == std::string("-") + short_name) {
            args.pop_back();
            return true;
        }

        if (alias.has_value()) {
            auto alias_v = alias.value();
            auto alias_dash = std::string(alias_v[1] == '\0' ? "-" : "--") + alias_v;
            if (to_check == alias_v || to_check == alias_dash) {
                args.pop_back();
                return true;
            }
        }

        return false;
    }

}
