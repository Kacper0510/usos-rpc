#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <tuple>

#include "../exceptions.hpp"
#include "utilities.hpp"

namespace usos_rpc::icalendar {

    /// @brief Represents a single event in the timetable, for example a lecture or a class.
    class Event {
        /// @brief Unique identifier of the event.
        std::string _uid;
        /// @brief University subject.
        std::string _subject;
        /// @brief Event type abbreviation, meaning for example a lecture or lab classes.
        std::optional<std::string> _type;
        /// @brief URL pointing at the event in the Web version of USOS.
        std::optional<std::string> _url;

        /// @brief Event location.
        struct Location {
            std::optional<std::string> room;
            std::optional<std::string> building;
            std::string address;
        } _location;

        /// @brief Date and time of the beginning of the event.
        std::chrono::local_seconds _start;
        /// @brief Date and time of the end of the event.
        std::chrono::local_seconds _end;

    public:
        Event() = delete;

        /// @brief Constructor based on VEVENT format.
        /// @param summary contains subject and type
        /// @param dtstart start of the event
        /// @param dtend end of the event
        /// @param uid identifier
        /// @param description contains partial location and URL
        /// @param location address
        /// @throws Exception when timestamp parsing fails
        Event(
            const std::string& summary,
            const std::string& dtstart,
            const std::string& dtend,
            const std::string& uid,
            const std::string& description,
            const std::string& location
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
                start_stream >> std::chrono::parse("%Y%m%dT%H%M%S", _start);
                std::istringstream end_stream(dtend);
                end_stream.exceptions(std::ios_base::badbit);
                end_stream >> std::chrono::parse("%Y%m%dT%H%M%S", _end);
            } catch (const std::ios_base::failure& err) {
                throw Exception(ExceptionType::PARSE_ERROR, "Could not parse event timestamp!");
            }

            auto description_parts = split(description, "\\n");
            if (description_parts.size() == 3) {
                auto room_split = split(description_parts[0], ": ");
                auto room = room_split.size() == 2 ? room_split[1] : description_parts[0];
                _location = { .room = room, .building = description_parts[1], .address = location };
                _url = description_parts[2];
            } else {
                _location = { .address = location };
            }
        }

        /// @brief Returns unique identifier of the event.
        [[nodiscard]]
        const std::string& uid() const {
            return _uid;
        }

        /// @brief Returns university subject.
        [[nodiscard]]
        const std::string& subject() const {
            return _subject;
        }

        /// @brief Returns event type abbreviation.
        [[nodiscard]]
        const std::optional<std::string>& type() const {
            return _type;
        }

        /// @brief Returns URL pointing at the event in the Web version of USOS.
        [[nodiscard]]
        const std::optional<std::string>& url() const {
            return _url;
        }

        /// @brief Returns event location.
        [[nodiscard]]
        const Location& location() const {
            return _location;
        }

        /// @brief Returns date and time of the beginning of the event.
        [[nodiscard]]
        const std::chrono::local_seconds& start() const {
            return _start;
        }

        /// @brief Returns date and time of the end of the event.
        [[nodiscard]]
        const std::chrono::local_seconds& end() const {
            return _end;
        }

        /// @brief Returns date and time of the beginning of the event as a zoned time.
        [[nodiscard]]
        const std::chrono::zoned_seconds start(const std::chrono::time_zone* tz) const {
            return std::chrono::zoned_seconds(tz, _start);
        }

        /// @brief Returns date and time of the end of the event as a zoned time.
        [[nodiscard]]
        const std::chrono::zoned_seconds end(const std::chrono::time_zone* tz) const {
            return std::chrono::zoned_seconds(tz, _end);
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
    };

}
