/// @file
/// @brief Code to be run when no other commands have been specified.

#pragma once

#include <csignal>
#include <cstdlib>
#include <string>

#include "exceptions.hpp"
#include "logging.hpp"

#include "discord_rpc.h"

namespace {

    /// @brief Callback for Discord RPC, invoked when connection to Discord instance is successful.
    /// @param user user logged in the currently running Discord instance
    void handle_discord_ready(const DiscordUser* user) {
        usos_rpc::lprint(usos_rpc::colors::SUCCESS, "Connected to Discord: {} ({})\n", user->username, user->userId);
    }

    /// @brief Callback for Discord RPC, invoked when connection to Discord instance is lost.
    /// @param error_code Discord error code
    /// @param message Discord error message
    void handle_discord_disconnected(int error_code, const char* message) {
        using namespace usos_rpc;
        throw Exception(ExceptionType::DISCORD, "Disconnected ({}) - {}", error_code, message);
    }

    /// @brief Callback for Discord RPC, invoked a Discord error occurs.
    /// @param error_code Discord error code
    /// @param message Discord error message
    void handle_discord_error(int error_code, const char* message) {
        using namespace usos_rpc;
        throw Exception(ExceptionType::DISCORD, "{} ({})", message, error_code);
    }

    /// @brief Custom Discord event handlers. Some are deliberately set to null as those events should not happen.
    DiscordEventHandlers handlers = {
        .ready = handle_discord_ready,
        .disconnected = handle_discord_disconnected,
        .errored = handle_discord_error,
        .joinGame = nullptr,
        .spectateGame = nullptr,
        .joinRequest = nullptr,
    };

    /// @brief Detects when Ctrl+C was pressed (SIGINT) or program should terminate (SIGTERM).
    volatile std::sig_atomic_t ctrl_c_detected = 0;

}

/// @brief Passed to std::signal() as SIGINT and SIGTERM handler.
/// @param signal ignored
extern "C" void ctrl_c_signal_handler(int signal) {
    ctrl_c_detected = 1;
}

namespace usos_rpc::commands {

    void run_default(const std::string& id) {
        std::signal(SIGINT, ctrl_c_signal_handler);
        std::signal(SIGTERM, ctrl_c_signal_handler);
        Discord_Initialize(id.c_str(), &handlers, false, nullptr);

        try {
            DiscordRichPresence presence = {
                .state = "test",
                .details = "¯\\_(ツ)_/¯",
                .startTimestamp = std::time(nullptr) + 60,
                .endTimestamp = std::time(nullptr) + 3 * 60,
                .largeImageKey = nullptr,
                .largeImageText = nullptr,
                .smallImageKey = nullptr,
                .smallImageText = nullptr,
                .partyId = nullptr,
                .partySize = 0,
                .partyMax = 0,
                .partyPrivacy = 0,
                .matchSecret = nullptr,
                .joinSecret = nullptr,
                .spectateSecret = nullptr,
                .instance = 0,
            };
            Discord_UpdatePresence(&presence);
            while (!ctrl_c_detected) {
                Discord_RunCallbacks();
            }
        } catch (...) {
            Discord_Shutdown();
            throw;
        }
        Discord_Shutdown();
        lprint(colors::SUCCESS, "Rich presence stopped successfully!\n");
    }

}
