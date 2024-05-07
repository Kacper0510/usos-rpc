/// @file
/// @brief Code to be run when no other commands have been specified.

#pragma once

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <string>

#include "../config.hpp"
#include "../exceptions.hpp"
#include "../files.hpp"
#include "../logging.hpp"
#include "../utilities.hpp"
#include "build_info.hpp"

#include "date/date.h"
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

    /// @brief Service loop contents.
    /// @param next time of next update
    void update_presence(std::chrono::time_point<std::chrono::system_clock>& next, usos_rpc::Config& config) {
        using namespace usos_rpc;

        auto now = std::chrono::system_clock::now();
        if (next < now) {
            try {
                lprint("Refreshing calendar data...\n");
                bool changed = config.refresh_calendar();
                if (changed) {
                    lprint(colors::SUCCESS, "Calendar data has been refreshed successfully:\n");
                    lprint("{}\n", config.calendar().name());
                } else {
                    lprint("Nothing has changed in the calendar since the last check.\n");
                }
            } catch (const Exception&) {
                eprint(colors::WARNING, "Calendar refresh failed!\n");
            }

            lprint(
                colors::OTHER,
                "Update interval reached at {}\n",
                date::format("%Y-%m-%d %H:%M", date::zoned_time(date::current_zone(), next))
            );

            auto maybe_event = config.calendar().next_event();
            if (maybe_event.has_value()) {
                auto event = maybe_event.value();
                if (event->start(config.calendar().time_zone()).get_sys_time() < now) {
                    Discord_UpdatePresence(config.create_presence_object(*event));
                    auto until_end = event->end(config.calendar().time_zone()).get_sys_time() - now;
                    next += min_duration(config.idle_refresh_rate(), until_end);
                    lprint("Current event:\n{}", *event);
                } else {
                    Discord_ClearPresence();
                    auto until_start = event->start(config.calendar().time_zone()).get_sys_time() - now;
                    next += min_duration(config.idle_refresh_rate(), until_start);
                }
                lprint(colors::SUCCESS, "Rich presence has been refreshed successfully!\n");
            } else {
                eprint(colors::WARNING, "No upcoming events were found!\n");
                Discord_ClearPresence();
                next += config.idle_refresh_rate();
            }
        }

        try {
            Discord_RunCallbacks();
        } catch (const Exception&) {}  // These exceptions are purely informational.
    }

    /// @brief Detects when Ctrl+C was pressed (SIGINT) or program should terminate (SIGTERM).
    volatile std::sig_atomic_t ctrl_c_detected = 0;

}

/// @brief Passed to std::signal() as SIGINT and SIGTERM handler.
/// @param signal ignored
extern "C" void ctrl_c_signal_handler(int signal) {
    ctrl_c_detected = 1;
}

namespace usos_rpc::commands {

    void run_default() {
        lprint(colors::OTHER, "USOS Discord Rich Presence {}\n", VERSION);

        lprint("Reading configuration file (in {})...\n", get_config_directory()->string());
        auto config = read_config();
        lprint(colors::SUCCESS, "Configuration file has been read successfully!\n");

        std::signal(SIGINT, ctrl_c_signal_handler);
        std::signal(SIGTERM, ctrl_c_signal_handler);
        Discord_Initialize(config.discord_app_id().c_str(), &handlers, false, nullptr);

        try {
            auto next_update = std::chrono::system_clock::now();
            while (!ctrl_c_detected) {
                update_presence(next_update, config);
            }
        } catch (...) {
            Discord_Shutdown();
            throw;
        }
        Discord_Shutdown();
        lprint(colors::SUCCESS, "Rich presence has been stopped successfully!\n");
    }

}
