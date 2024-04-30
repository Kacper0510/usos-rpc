#include <string>

#include "build_info.hpp"
#include "commands/info.hpp"
#include "commands/parser.hpp"
#include "exceptions.hpp"
#include "files.hpp"
#include "icalendar/parser.hpp"
#include "logging.hpp"
#include "preinit.hpp"
#include "requests.hpp"
#include "utilities.hpp"

/// @brief Entry point of the program.
/// @param argc number of command line arguments
/// @param argv command line arguments
/// @return process return code
int main(const int argc, const char* argv[]) {
    using namespace usos_rpc;

    auto args = commands::create_arguments_vector(argc, argv);

    if (commands::check_command(args, "version")) {
        commands::version();
        return 0;
    } else if (commands::check_command(args, "config")) {
        commands::config();
        return 0;
    } else if (commands::check_command(args, "help", "?")) {
        WindowsConsole::enable_features();
        commands::help();
        return 0;
    }

    initialize();

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
