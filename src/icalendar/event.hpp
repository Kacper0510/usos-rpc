/// @file
/// @brief iCalendar event representation.

#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <variant>

#include "../exceptions.hpp"
#include "utilities.hpp"

#include "date/date.h"
#include "date/tz.h"
#include "fmt/color.h"
#include "fmt/format.h"

namespace usos_rpc::icalendar {

    /// @brief Represents a single event in the timetable, for example a lecture or a class.
    class Event {
        /// @brief Unique identifier of the event.
        std::string _uid;
        /// @brief University subject.
        std::string _subject;
        /// @brief Event type abbreviation, meaning for example a lecture or lab classes.
        std::optional<std::string> _type;
        /// @brief URL pointing at the event in the web version of USOS.
        std::optional<std::string> _url;

        /// @brief Full event location structure.
        struct FullLocation {
            std::string room;
            std::string building;
            std::string address;
        };

        using Location = std::variant<std::monostate, std::string, FullLocation>;
        /// @brief Event location. Can be one of three variants:
        /// std::monostate - no location was found,
        /// std::string - only the address was parsed successfully,
        /// usos_rpc::Event::FullLocation - location was fully parsed.
        Location _location;

        /// @brief Date and time of the beginning of the event.
        date::local_seconds _start;
        /// @brief Date and time of the end of the event.
        date::local_seconds _end;

    public:
        Event() = delete;

        /// @brief Constructor based on VEVENT format.
        /// @param summary contains subject and type
        /// @param dtstart start of the event
        /// @param dtend end of the event
        /// @param uid identifier
        /// @param description contains partial location and URL
        /// @param location optional address
        /// @throws usos_rpc::Exception when timestamp parsing fails
        Event(
            const std::string& summary,
            const std::string& dtstart,
            const std::string& dtend,
            const std::string& uid,
            const std::string& description,
            const std::optional<std::string>& location
        ):
        _uid(uid) {
            auto summary_parts = split(summary, " - ");
            switch (summary_parts.size()) {
                case 2:
                    _subject = summary_parts[1];
                    _type = summary_parts[0];
                    break;
                case 1:
                    _subject = summary_parts[0];
                    break;
                default:
                    _subject = summary;
                    break;
            }

            try {
                std::istringstream start_stream(dtstart);
                start_stream.exceptions(std::ios_base::badbit);
                start_stream >> date::parse("%Y%m%dT%H%M%S", _start);
                std::istringstream end_stream(dtend);
                end_stream.exceptions(std::ios_base::badbit);
                end_stream >> date::parse("%Y%m%dT%H%M%S", _end);
            } catch (const std::ios_base::failure& err) {
                throw Exception(ExceptionType::ICALENDAR, "Could not parse event timestamp!");
            }

            if (!location.has_value()) {
                return;
            }

            auto description_parts = split(description, "\n");
            std::erase_if(description_parts, [](const auto& value) {
                return strip(value) == "";
            });
            if (description_parts.size() == 3) {
                auto room_split = split(description_parts[0], ": ");
                auto room = room_split.size() == 2 ? room_split[1] : description_parts[0];
                _location =
                    FullLocation { .room = room, .building = description_parts[1], .address = location.value() };
                _url = description_parts[2];
            } else {
                _location = std::string(location.value());
            }
        }

        /// @brief Returns unique identifier of the event.
        /// @return event unique identifier
        [[nodiscard]]
        const std::string& uid() const {
            return _uid;
        }

        /// @brief Returns university subject.
        /// @return event subject
        [[nodiscard]]
        const std::string& subject() const {
            return _subject;
        }

        /// @brief Returns event type abbreviation.
        /// @return event type
        [[nodiscard]]
        const std::optional<std::string>& type() const {
            return _type;
        }

        /// @brief Returns URL pointing at the event in the Web version of USOS.
        /// @return event URL
        [[nodiscard]]
        const std::optional<std::string>& url() const {
            return _url;
        }

        /// @brief Returns address if the event has one.
        /// @return event address or nullptr
        [[nodiscard]]
        const std::string* address() const {
            if (std::holds_alternative<std::string>(_location)) {
                return &std::get<std::string>(_location);
            } else if (std::holds_alternative<FullLocation>(_location)) {
                return &std::get<FullLocation>(_location).address;
            }
            return nullptr;
        }

        /// @brief Returns room if the event has one.
        /// @return event room or nullptr
        [[nodiscard]]
        const std::string* room() const {
            if (std::holds_alternative<FullLocation>(_location)) {
                return &std::get<FullLocation>(_location).room;
            }
            return nullptr;
        }

        /// @brief Returns building if the event has one.
        /// @return event building or nullptr
        [[nodiscard]]
        const std::string* building() const {
            if (std::holds_alternative<FullLocation>(_location)) {
                return &std::get<FullLocation>(_location).building;
            }
            return nullptr;
        }

        /// @brief Checks whether the event has full location information (i.e. building, room and address data).
        /// @return result of the check
        [[nodiscard]]
        bool has_full_location() const {
            return std::holds_alternative<FullLocation>(_location);
        }

        /// @brief Returns date and time of the beginning of the event.
        /// @return start of the event
        [[nodiscard]]
        const date::local_seconds& start() const {
            return _start;
        }

        /// @brief Returns date and time of the end of the event.
        /// @return end of the event
        [[nodiscard]]
        const date::local_seconds& end() const {
            return _end;
        }

        /// @brief Returns date and time of the beginning of the event as a zoned time.
        /// @return zoned start of the event
        [[nodiscard]]
        const date::zoned_seconds start(const date::time_zone* tz) const {
            return date::zoned_seconds(tz, _start);
        }

        /// @brief Returns date and time of the end of the event as a zoned time.
        /// @return zoned end of the event
        [[nodiscard]]
        const date::zoned_seconds end(const date::time_zone* tz) const {
            return date::zoned_seconds(tz, _end);
        }

        /// @brief Compares by unique identifier.
        /// @param other event to compare
        /// @return true if equal
        bool operator==(const Event& other) const {
            return _uid == other._uid;
        }

        /// @brief Compares by time of start, then end, then unique identifier.
        /// @param other event to compare
        /// @return comparison result
        auto operator<=>(const Event& other) const {
            return std::tie(_start, _end, _uid) <=> std::tie(other._start, other._end, other._uid);
        }

        /// @brief Event formatting support for fmt.
        /// @param event event to format
        /// @return formatted string
        friend auto format_as(const Event& event) {
            auto start = date::format("%Y-%m-%d %H:%M", event._start);
            auto end = date::format("%H:%M", event._end);

            std::string location;
            if (event.has_full_location()) {
                location = fmt::format(
                    "{} - {}, {}",
                    std::get<Event::FullLocation>(event._location).room,
                    std::get<Event::FullLocation>(event._location).building,
                    std::get<Event::FullLocation>(event._location).address
                );
            } else if (std::holds_alternative<std::string>(event._location)) {
                location = std::get<std::string>(event._location);
            }

            return fmt::format(
                "{} - {} ({} - {}):\n{}\n",
                fmt::styled(event._subject, colors::OTHER),
                event._type.value_or("???"),
                start,
                end,
                location
            );
        }
    };

}
