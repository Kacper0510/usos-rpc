/// @file
/// @brief iCalendar file structure representation.

#pragma once

#include <format>
#include <chrono>
#include <optional>
#include <set>
#include <string>

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
        std::set<Event> _events;

    public:
        Calendar() = default;  // To simplify Config class.

        /// @brief Constructor based on VCALENDAR format.
        /// @param events list of events
        Calendar(
            const std::string& calname,
            const std::string& prodid,
            const std::string& timezone,
            const std::set<Event>& events
        ):
        _name(calname),
        _product_id(prodid),
        _events(events) {
            _time_zone = std::chrono::locate_zone(timezone);
        }

        /// @brief Returns an iterator to the current/upcoming event, or nullopt if none were found.
        /// In order to do that, additionally deletes events from the past from the list of events.
        /// @return pointer (iterator) to the next event or nullopt
        std::optional<std::set<Event>::iterator> next_event() {
            auto now = std::chrono::system_clock::now();
            auto it = _events.begin();
            while (it != _events.end()) {
                if (it->end(_time_zone).get_sys_time() < now) {
                    it = _events.erase(it);
                } else {
                    return it;
                }
            }
            return std::nullopt;
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

        /// @brief Returns the set of events.
        /// @return list of events
        [[nodiscard]]
        const std::set<Event>& events() const {
            return _events;
        }

        friend struct std::formatter<Calendar>;
    };

}

namespace std {

    template <>
    struct formatter<usos_rpc::icalendar::Calendar> {
        constexpr auto parse(format_parse_context& ctx) {
            auto it = ctx.begin();
            if (it != ctx.end() && *it != '}') {
                throw format_error("Invalid format specifier for Calendar");
            }
            return it;
        }

        auto format(const usos_rpc::icalendar::Calendar& event, format_context& ctx) const {
            string joined_events;
            bool first = true;
            for (const auto& ev : event._events) {
                if (!first) {
                    joined_events.push_back('\n');
                }
                first = false;
                std::format_to(back_inserter(joined_events), "{}", ev);
            }

            return std::format_to(
                ctx.out(),
                "{}\nProduct ID: {}\nTime zone: {}\nEvents:\n\n{}",
                usos_rpc::styled(event._name, usos_rpc::TextStyle::OTHER),
                event._product_id,
                event._time_zone->name(),
                joined_events
            );
        }
    };

}
