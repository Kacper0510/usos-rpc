/// @file
/// @brief Configuration file structure definition.

#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <regex>
#include <string>

#include "exceptions.hpp"
#include "files.hpp"
#include "icalendar/calendar.hpp"
#include "icalendar/parser.hpp"
#include "requests.hpp"

#include "discord_rpc.h"
#include "toml++/toml.hpp"

namespace {

    /// @brief Regular expression for a valid Discord identifier (uint64_t).
    const std::regex DISCORD_ID(R"(\d{1,20})");
    /// @brief Regular expression for removing DTSTAMP occurences.
    const std::regex DTSTAMP(R"(\nDTSTAMP;VALUE=DATE-TIME:\d{8}T\d{6}Z?\r?\n)");

}

namespace usos_rpc {

    /// @brief Represents config.toml structure. For more info, open the default file in resources directory.
    class Config {
        /// @brief iCalendar file path or http/webcal link.
        std::string _calendar_location;
        /// @brief Discord Rich Presence application identifier.
        /// @see https://discord.com/developers/applications
        std::string _discord_app_id;
        /// @brief Calendar data refresh rate when idle (no upcoming events in the nearest future).
        std::int64_t _idle_refresh_rate = 30;

        /// @brief Temporary solution for global large image key.
        std::optional<std::string> _image_key;

        /// @brief iCalendar file hash.
        std::size_t _calendar_hash;
        /// @brief Parsed calendar structure.
        icalendar::Calendar _calendar;

    public:
        /// @brief Constructs an object based on parsed TOML data.
        /// @param parsed_file TOML data for config.toml
        /// @throws usos_rpc::Exception when the necessary properties are invalid or not found
        explicit Config(const toml::table& parsed_file): _calendar_hash() {
            auto cal = parsed_file.get_as<std::string>("calendar");
            if (!cal || (_calendar_location = cal->get()).size() == 0) {
                throw Exception(ExceptionType::CONFIG, "Empty 'calendar' property! Please fix the config file.");
            }

            auto raw_app_id = parsed_file.get("discord_app_id");
            if (!raw_app_id) {
                throw Exception(ExceptionType::CONFIG, "Empty 'discord_app_id' property! Please fix the config file.");
            } else if (raw_app_id->is<std::int64_t>()) {
                auto app_id = raw_app_id->value<std::int64_t>().value();
                if (app_id <= 0) {
                    throw Exception(
                        ExceptionType::CONFIG, "Invalid 'discord_app_id' property! Please fix the config file."
                    );
                }
                _discord_app_id = std::to_string(app_id);
            } else if (raw_app_id->is<std::string>()) {
                _discord_app_id = raw_app_id->value<std::string>().value();
                if (_discord_app_id.size() == 0 || !std::regex_match(_discord_app_id, DISCORD_ID)) {
                    throw Exception(
                        ExceptionType::CONFIG, "Invalid 'discord_app_id' property! Please fix the config file."
                    );
                }
            } else {
                throw Exception(
                    ExceptionType::CONFIG,
                    "Wrong type of 'discord_app_id' property! Please change it to a string or an integer."
                );
            }

            auto refresh = parsed_file.get_as<std::int64_t>("idle_refresh_rate");
            if (refresh && refresh->get() > 0) {
                _idle_refresh_rate = refresh->get();
            }

            auto key = parsed_file.get_as<std::string>("image_key");
            if (key && key->get().size() > 0) {
                _image_key = key->get();
            }
        }

        /// @brief Refreshes cached calendar structure based on the given link if its hash has changed.
        /// @return true if refresh was necessary, false if nothing has changed
        /// @throws usos_rpc::Exception when reading or parsing calendar data fails
        bool refresh_calendar() {
            auto cal_raw = fetch_content(_calendar_location);
            // Remove DTSTAMP properties because they always change and mess up hashing.
            auto cal = std::regex_replace(cal_raw, DTSTAMP, "\n");

            auto new_hash = std::hash<std::string> {}(cal);
            if (new_hash != _calendar_hash) {
                _calendar = icalendar::parse(cal);
                _calendar_hash = new_hash;
                return true;
            }
            return false;
        }

        /// @brief Creates Discord Rich Presence representation based on given event.
        /// @param event current event
        /// @return Rich Presence object
        [[nodiscard]]
        const DiscordRichPresence* create_presence_object(const usos_rpc::icalendar::Event& event) {
            // Caching objects to not create dangling pointers
            static std::string subject;
            static std::optional<std::string> location;
            static DiscordRichPresence presence;

            subject = event.type().has_value()
                ? fmt::format("{} - {}", event.subject(), event.type().value())
                : event.subject();
            location = event.has_full_location()
                ? std::optional(fmt::format("{} - {}", *event.room(), *event.building()))
                : std::nullopt;
            presence = {
                .state = location.transform(&std::string::c_str).value_or(nullptr),
                .details = subject.c_str(),
                .startTimestamp = event.start(_calendar.time_zone()).get_sys_time().time_since_epoch().count(),
                .endTimestamp = event.end(_calendar.time_zone()).get_sys_time().time_since_epoch().count(),
                .largeImageKey = _image_key.transform(&std::string::c_str).value_or(nullptr),
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

            return &presence;
        }

        /// @brief Returns parsed calendar structure, cached in this object.
        [[nodiscard]]
        const icalendar::Calendar& calendar() const {
            return _calendar;
        }

        /// @brief Returns parsed calendar structure, cached in this object.
        [[nodiscard]]
        icalendar::Calendar& calendar() {
            return _calendar;
        }

        /// @brief Returns chosen idle calendar refresh rate.
        [[nodiscard]]
        std::chrono::minutes idle_refresh_rate() const {
            return std::chrono::minutes(_idle_refresh_rate);
        }

        /// @brief Returns chosen calendar path/link.
        [[nodiscard]]
        const std::string& calendar_location() const {
            return _calendar_location;
        }

        /// @brief Returns chosen Discord app identifier.
        [[nodiscard]]
        const std::string& discord_app_id() const {
            return _discord_app_id;
        }
    };

    /// @brief Reads and parses config.toml.
    /// @return parsed Config instance
    /// @throws usos_rpc::Exception when reading or parsing the file fails
    Config read_config() {
        auto path = *get_config_directory() / "config.toml";
        auto contents = read_file(path.string());
        try {
            auto table = toml::parse(contents);
            return Config(table);
        } catch (const toml::parse_error& e) {
            throw Exception(
                ExceptionType::CONFIG,
                "{} at line {}, column {}, in {}",
                e.description(),
                e.source().begin.line,
                e.source().begin.column,
                path.string()
            );
        }
    }
}
