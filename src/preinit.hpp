#pragma once

#include <filesystem>
#include <memory>

#include "exceptions.hpp"
#include "files.hpp"
#include "logging.hpp"
#include "preinit_files.hpp"

#include "date/date.h"

#ifdef _WIN32
    #include "windows.h"
#endif

namespace usos_rpc {

    // clang-format off
    #ifdef _WIN32
    
        /// @brief Windows console encoding and ASCII codes setup class.
        class WindowsConsole {
            /// @brief Contains previous standard output console mode.
            DWORD _console_mode_stdout = 0;
            /// @brief Contains previous standard output console mode.
            DWORD _console_mode_stderr = 0;
            /// @brief Contains previous console code page.
            UINT _code_page = 0;

            /// @brief Whether the features of this class are enabled.
            bool _enabled = false;

            /// @brief Singleton instance.
            static WindowsConsole _instance;

        public:
            /// @brief Enables all features.
            static void enable_features() {
                if (_instance._enabled) {
                    return;
                }

                if (::usos_rpc::should_show_colored_output()) {
                    _instance.enable_colors();
                }
                _instance.enable_unicode();
                _instance._enabled = true;
            }

        private:
            WindowsConsole() {}

            WindowsConsole(const WindowsConsole&) = delete;
            void operator=(const WindowsConsole&) = delete;

            /// @brief Disables all features.
            ~WindowsConsole() {
                if (_enabled) {
                    disable_unicode();
                    if (::usos_rpc::should_show_colored_output()) {
                        disable_colors();
                    }
                }
            }

            /// @brief Saves current console mode and enables virtual terminal processing (ASCII color codes).
            void enable_colors() {
                HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
                GetConsoleMode(out, &_console_mode_stdout);
                SetConsoleMode(out, _console_mode_stdout | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

                HANDLE err = GetStdHandle(STD_ERROR_HANDLE);
                GetConsoleMode(err, &_console_mode_stderr);
                SetConsoleMode(err, _console_mode_stderr | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }

            /// @brief Reverts previous console mode.
            void disable_colors() const {
                HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleMode(out, _console_mode_stdout);

                HANDLE err = GetStdHandle(STD_ERROR_HANDLE);
                SetConsoleMode(err, _console_mode_stderr);
            }

            /// @brief Saves current console encoding and then changes it to Unicode.
            void enable_unicode() {
                _code_page = GetConsoleOutputCP();
                SetConsoleOutputCP(CP_UTF8);
            }

            /// @brief Reverts previous console encoding.
            void disable_unicode() const {
                SetConsoleOutputCP(_code_page);
            }
        };

        // static variable initialization
        WindowsConsole WindowsConsole::_instance {};
    
    #endif
    // clang-format on

    /// @brief Asserts that all necessary files exist in the config folder.
    /// Otherwise, it creates the files from cache embedded in the executable.
    void assert_files() {
        auto config_dir = *get_config_directory();
        for (const auto& [name, data] : EMBEDDED_FILES) {
            auto file = config_dir / name;
            if (!std::filesystem::exists(file)) {
                std::filesystem::create_directories(file.parent_path());
                write_file(file.string(), data);
            }
        }
    }

    /// @brief Initializes config directory.
    /// On Windows also initializes 'date' library and sets up console.
    /// @throws Exception when file operations fail.
    void initialize() {  // clang-format off
        try {
            assert_files();
        } catch (const std::filesystem::filesystem_error& err) {
            throw Exception(ExceptionType::IO_ERROR, err.code().message());
        }

        #ifdef _WIN32
            auto tzdata_path = *get_config_directory() / "tzdata";
            date::set_install(tzdata_path.string());

            WindowsConsole::enable_features();
        #endif
    }  // clang-format on

}
