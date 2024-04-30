#pragma once

#include <algorithm>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include "../exceptions.hpp"
#include "../utilities.hpp"
#include "calendar.hpp"
#include "event.hpp"

namespace {

    /// @brief Splits the iCalendar text into singular lines/properties.
    /// @param text text of an iCalendar file
    /// @return preprocessed vector of properties
    [[nodiscard]]
    std::vector<std::string> preprocess(const std::string& text) {
        auto lines = usos_rpc::split(text, "\n");
        auto iter = lines.begin();
        while (iter != lines.end()) {
            auto current = usos_rpc::strip(*iter);
            if (current.empty()) {
                iter = lines.erase(iter);
                continue;
            }

            if (iter->starts_with(" ")) {
                auto previous = iter - 1;
                previous->append(current);
                iter = lines.erase(iter);
            } else {
                *iter = current;
                iter++;
            }
        }
        return lines;
    }

    /// @brief Regular expression for backslash-n sequence.
    const std::regex NEWLINE(R"(\\n)");
    /// @brief Regular expression for backslash-comma sequence.
    const std::regex COMMA(R"(\\,)");

    /// @brief Fixes escaped characters in the iCalendar text.
    /// @param lines vector of properties
    void fix_escapes(std::vector<std::string>& lines) {
        std::for_each(lines.begin(), lines.end(), [](std::string& line) {
            line = std::regex_replace(line, NEWLINE, "\n");
            line = std::regex_replace(line, COMMA, ",");
        });
    }

    using lines_iterator = std::vector<std::string>::const_iterator;

    /// @brief Extracts a property from the iCalendar text.
    /// @param begin beginning of the line range
    /// @param end end of the line range
    /// @param name property name
    /// @return property value
    /// @throws Exception when the property is missing
    [[nodiscard]]
    std::string get_property(const lines_iterator& begin, const lines_iterator& end, const std::string& name) {
        auto iter = std::find_if(begin, end, [&name](const std::string& line) {
            return line.starts_with(name);
        });
        if (iter == end) {
            throw usos_rpc::Exception(usos_rpc::ExceptionType::ICALENDAR, "Missing property: {}", name);
        }
        return iter->substr(name.size() + 1);
    }

    /// @brief Extracts a property from the iCalendar text.
    /// @param lines vector of properties
    /// @param name property name
    /// @return property value
    /// @throws Exception when the property is missing
    [[nodiscard]]
    std::string get_property(const std::vector<std::string>& lines, const std::string& name) {
        return get_property(lines.begin(), lines.end(), name);
    }

}

namespace usos_rpc::icalendar {

    /// @brief Parses given text into a Calendar object.
    /// @param text text of an iCalendar file
    /// @return parsed calendar data
    /// @throws Exception when parsing fails
    [[nodiscard]]
    Calendar parse(std::string text) {
        auto lines = preprocess(text);
        fix_escapes(lines);
        if (lines.empty() || !lines.front().starts_with("BEGIN:VCALENDAR")
            || !lines.back().starts_with("END:VCALENDAR")) {
            throw Exception(ExceptionType::ICALENDAR, "Invalid iCalendar file!");
        }

        bool event_fail = false;
        std::set<Event> events;
        auto iter = lines.begin() + 1;
        while (iter != lines.end() - 1) {
            if (iter->starts_with("BEGIN:VEVENT")) {
                auto end = std::find(iter, lines.end() - 1, "END:VEVENT");

                try {
                    auto dtstart = get_property(iter, end, "DTSTART").substr(const_string_length("VALUE=DATE-TIME:"));
                    auto dtend = get_property(iter, end, "DTEND").substr(const_string_length("VALUE=DATE-TIME:"));
                    Event event(
                        get_property(iter, end, "SUMMARY"),
                        dtstart,
                        dtend,
                        get_property(iter, end, "UID"),
                        get_property(iter, end, "DESCRIPTION"),
                        get_property(iter, end, "LOCATION")
                    );
                    events.insert(event);
                } catch (const Exception& err) {
                    event_fail = true;
                }

                iter = end + 1;
            } else {
                iter++;
            }
        }
        if (event_fail && events.empty()) {
            throw Exception(ExceptionType::ICALENDAR, "Could not parse events!");
        }

        auto prodid = get_property(lines, "PRODID");
        auto calname = get_property(lines, "X-WR-CALNAME");
        auto timezone = get_property(lines, "X-WR-TIMEZONE");
        return Calendar(calname, prodid, timezone, events);
    }

}
