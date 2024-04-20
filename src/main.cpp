#include <string>

#include "exceptions.hpp"
#include "files.hpp"
#include "icalendar/parser.hpp"
#include "requests.hpp"
#include "version_info.hpp"

#include "curl/curl.h"
#include "fmt/core.h"

/// @brief Entry point of the program.
/// @return process return code
int main() {
    // this function currently only tests some features

    fmt::println("USOS Rich Presence v{}", usos_rpc::VERSION);
    fmt::println("Curl: {}", curl_version());
    fmt::println("User agent: {}", usos_rpc::USER_AGENT);

    auto response = usos_rpc::http_get("https://api.ipify.org/");
    fmt::println("IP: {}", response);

    fmt::println("Home path: {}", usos_rpc::get_config_directory()->string());

    try {
        auto ics_file = usos_rpc::read_file("test.ics");
        auto calendar = usos_rpc::icalendar::parse(ics_file);
        fmt::println("Calendar:\n\n{}", calendar);
    } catch (const usos_rpc::Exception& e) {
        fmt::print(stderr, "Error: {}", e);
        return 1;
    }

    return 0;
}
