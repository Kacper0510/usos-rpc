#pragma once

#include <chrono>
#include <string>

#include "utilities.hpp"

namespace usos_rpc::icalendar {

    /// @brief Represents a single event in the timetable, for example a lecture or a class.
    class Event {
        /// @brief Unique identifier of the event.
        std::string uid;
        /// @brief University subject.
        std::string subject;
        /// @brief Event type abbreviation, meaning for example a lecture or lab classes.
        std::string type;
        /// @brief URL pointing at the event in the Web version of USOS.
        std::string url;

        /// @brief Event location.
        struct Location {
            std::string room;
            std::string building;
            std::string address;
        } location;

        /// @brief Date and time of the beginning of the event.
        std::chrono::local_seconds start;
        /// @brief Date and time of the end of the event.
        std::chrono::local_seconds end;

    public:

        /// @brief Constructor based on VEVENT format.
        /// @param summary contains subject and type
        /// @param dtstart start of the event
        /// @param dtend end of the event
        /// @param uid identifier
        /// @param description contains partial location and URL
        /// @param location address
        Event(
            const std::string& summary,
            const std::string& dtstart,
            const std::string& dtend,
            const std::string& uid,
            const std::string& description,
            const std::string& location
        ):
        uid(uid) {
            auto summary_parts = split(summary, " - ");
            switch (summary_parts.size()) {
                case 2:
                    subject = summary_parts[1];
                    type = summary_parts[0];
                    break;
                case 1:
                    subject = summary_parts[0];
                    type = "?";
                    break;
                default:
                    subject = summary;
                    type = "?";
                    break;
            }

            std::istringstream start_stream(dtstart);
            start_stream.exceptions(std::ios_base::badbit);
            start_stream >> std::chrono::parse("%Y%m%dT%H%M%S", start);
            std::istringstream end_stream(dtend);
            end_stream.exceptions(std::ios_base::badbit);
            end_stream >> std::chrono::parse("%Y%m%dT%H%M%S", end);

            auto description_parts = split(description, "\n");
            if (description_parts.size() == 3) {
                auto room_split = split(description_parts[0], ": ");
                auto room = room_split.size() == 2 ? room_split[1] : description_parts[0];
                this->location = { .room = room, .building = description_parts[1], .address = location };
                url = description_parts[2];
            } else {
                throw std::string("Invalid description format!");
            }
        }

        /// @brief Copy constructor.
        /// @param other event to copy
        explicit Event(const Event& other) = default;
    };

}
