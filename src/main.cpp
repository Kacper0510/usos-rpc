/// @file
/// @brief Entry point of the program.

#include <exception>
#include <string>
#include <vector>

#include "commands/default.hpp"
#include "commands/extract.hpp"
#include "commands/info.hpp"
#include "exceptions.hpp"
#include "logging.hpp"
#include "preinit.hpp"
#include "utilities.hpp"

#ifdef _WIN32
    #include "commands/windows.hpp"
#else
    #include "commands/systemd.hpp"
#endif

/// @brief De facto true main function. Chooses appropriate action based on given command, if any.
/// @param args reversed command line argument list
void choose_command(std::vector<std::string>& args) {  // clang-format off
    using namespace usos_rpc;

    if (commands::check_command(args, "version")) {
        commands::version();
        return;
    } else if (commands::check_command(args, "config")) {
        commands::config();
        return;
    } else if (commands::check_command(args, "help", "?")) {
        commands::help();
        return;
    }

    #ifdef _WIN32  
        if (commands::check_command(args, "install")) {
            commands::install();
            return;
        } else if (commands::check_command(args, "uninstall")) {
            commands::uninstall();
            return;
        } else if (commands::check_command(args, "background")) {
            WindowsConsole::hide_permanently();
        }
    #else
        if (using_systemd()) {
            if (commands::check_command(args, "install")) {
                commands::install();
                return;
            } else if (commands::check_command(args, "uninstall")) {
                commands::uninstall();
                return;
            }
        }
    #endif

    initialize_config();
    commands::run_default();
}  // clang-format on

/// @brief Entry point of the program.
/// @param argc number of command line arguments
/// @param argv command line arguments
/// @return process return code
int main(const int argc, const char* argv[]) {
    try {
        usos_rpc::WindowsConsole::enable_features();
        auto args = usos_rpc::commands::create_arguments_vector(argc, argv);
        choose_command(args);
    } catch (const usos_rpc::Exception& e) {
        usos_rpc::eprint(usos_rpc::colors::FATAL_ERROR, "Fatal error - {}\n", e);
        return 1;
    } catch (const std::exception& e) {
        usos_rpc::eprint(usos_rpc::colors::FATAL_ERROR, "Fatal error: {}\n", e.what());
        return 1;
    } catch (...) {
        usos_rpc::eprint(usos_rpc::colors::FATAL_ERROR, "Unknown fatal error!\n");
        return 1;
    }
    return 0;
}
