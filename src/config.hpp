/// @file
/// @brief Configuration file structure definition.

#pragma once

#include <string>

#include "exceptions.hpp"
#include "files.hpp"
#include "icalendar/calendar.hpp"
#include "icalendar/parser.hpp"
#include "requests.hpp"

#include "toml++/toml.hpp"

namespace usos_rpc {

    /// @brief Represents config.toml structure. For more info, open the default file in resources directory.
    class Config {
        /// @brief iCalendar file path or http/webcal link.
        std::string _calendar_location;
        /// @brief Discord Rich Presence application identifier.
        /// @see https://discord.com/developers/applications
        std::string _discord_app_id;

        /// @brief iCalendar file hash.
        std::size_t _calendar_hash;
        /// @brief Parsed calendar structure.
        icalendar::Calendar _calendar;

    public:
        /// @brief Constructs an object based on parsed TOML data.
        /// @param parsed_file TOML data for config.toml
        /// @throws usos_rpc::Exception when the necessary properties are invalid or not found,
        /// when reading or parsing calendar data fails
        explicit Config(const toml::table& parsed_file): _calendar_hash() {
            auto cal = parsed_file.get_as<std::string>("calendar");
            if (!cal || (_calendar_location = cal->get()).size() == 0) {
                throw Exception(ExceptionType::CONFIG, "Empty 'calendar' property! Please fix the config file.");
            }
            auto app_id = parsed_file.get_as<std::string>("discord_app_id");
            if (!app_id || (_discord_app_id = app_id->get()).size() == 0) {
                throw Exception(
                    ExceptionType::CONFIG, "Invalid 'discord_app_id' property! Please fix the config file."
                );
            }
            refresh_calendar();
        }

        /// @brief Refreshes cached calendar structure based on the given link if its hash has changed.
        /// @return new or cached calendar object
        /// @throws usos_rpc::Exception when reading or parsing calendar data fails
        const icalendar::Calendar& refresh_calendar() {
            auto cal = fetch_content(_calendar_location);
            auto new_hash = std::hash<std::string> {}(cal);
            if (new_hash != _calendar_hash) {
                _calendar = icalendar::parse(cal);
                _calendar_hash = new_hash;
            }
            return _calendar;
        }

        /// @brief Returns parsed calendar structure, cached in this object.
        [[nodiscard]]
        const icalendar::Calendar& calendar() const {
            return _calendar;
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

}
