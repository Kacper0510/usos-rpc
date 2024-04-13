#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "event.hpp"

namespace usos_rpc::icalendar {

    /// @brief Represents a calendar/timetable containing events, parsed from iCalendar format.
    class Calendar {
        /// @brief Calendar name.
        std::string _name;
        /// @brief Product identifier of the software that generated this calendar file.
        std::string _product_id;
        /// @brief Calendar time zone, applied to all event timestamps.
        const std::chrono::time_zone* _time_zone;
        /// @brief List of events.
        std::vector<Event> _events;

    public:

        Calendar() = delete;

        /// @brief Constructor based on VCALENDAR format.
        /// @param events list of events
        explicit Calendar(
            const std::string& calname,
            const std::string& prodid,
            const std::string& timezone,
            const std::vector<Event>& events
        ):
        _name(calname),
        _product_id(prodid),
        _events(events) {
            _time_zone = std::chrono::locate_zone(timezone);
        }

        /// @brief Returns the calendar name.
        /// @return calendar name
        [[nodiscard]]
        const std::string& name() const {
            return _name;
        }

        /// @brief Returns the product identifier of the software that generated this calendar file.
        /// @return product identifier
        [[nodiscard]]
        const std::string& product_id() const {
            return _product_id;
        }

        /// @brief Returns the calendar time zone.
        /// @return time zone
        [[nodiscard]]
        const std::chrono::time_zone* time_zone() const {
            return _time_zone;
        }

        /// @brief Returns the list of events.
        /// @return list of events
        [[nodiscard]]
        const std::vector<Event>& events() const {
            return _events;
        }
    };
}