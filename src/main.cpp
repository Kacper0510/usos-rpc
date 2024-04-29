#include <string>

#include "exceptions.hpp"
#include "files.hpp"
#include "icalendar/parser.hpp"
#include "logging.hpp"
#include "preinit.hpp"
#include "requests.hpp"
#include "version_info.hpp"

#include "curl/curl.h"

/// @brief Entry point of the program.
/// @return process return code
int main() {
    usos_rpc::initialize();

    // this function currently only tests some features

    usos_rpc::print(fmt::fg(fmt::color::red) | fmt::emphasis::italic, "USOS Rich Presence v{}\n", usos_rpc::VERSION);
    usos_rpc::print("Curl: {}\n", curl_version());
    usos_rpc::print("User agent: {}\n", usos_rpc::USER_AGENT);

    auto response = usos_rpc::http_get("https://api.ipify.org/");
    usos_rpc::print("IP: {}\n", response);

    usos_rpc::print("Home path: {}\n", usos_rpc::get_config_directory()->string());

    try {
        auto ics_file = usos_rpc::read_file("test.ics");
        auto calendar = usos_rpc::icalendar::parse(ics_file);
        usos_rpc::print("Calendar:\n\n{}", calendar);
    } catch (const usos_rpc::Exception& e) {
        usos_rpc::eprint(fmt::fg(fmt::terminal_color::red), "Fatal error - {}", e);
        return 1;
    }

    return 0;
}
