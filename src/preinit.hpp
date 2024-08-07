/// @file
/// @brief Pre-initialization functions for config directory and Windows console.

#pragma once

#include <filesystem>
#include <memory>

#include "build_info.hpp"
#include "exceptions.hpp"
#include "files.hpp"
#include "logging.hpp"

#include "date/date.h"
#include "date/tz.h"

#ifdef _WIN32
    #include <windows.h>
#endif

namespace usos_rpc {

    // clang-format off
    #ifdef _WIN32
    
        /// @brief Windows console encoding and ANSI codes setup class. No-op on other platforms.
        class WindowsConsole {
            /// @brief Contains previous standard output console mode.
            DWORD _console_mode_stdout = 0;
            /// @brief Contains previous standard output console mode.
            DWORD _console_mode_stderr = 0;
            /// @brief Contains previous console code page.
            UINT _code_page = 0;

            /// @brief Whether the features of this class are enabled.
            enum class State {
                /// @brief Default console state after app startup or just before exit.
                DEFAULT,
                /// @brief Indicates that some changes that should be reverted were made to the console setup.
                MODIFIED,
                /// @brief The console was disabled/hidden with Windows API.
                FREED,
            } _state = State::DEFAULT;

            /// @brief Singleton instance.
            static WindowsConsole _instance;

        public:
            /// @brief Enables all features.
            static void enable_features() {
                if (_instance._state != State::DEFAULT) {
                    return;
                }

                if (should_show_colored_output()) {
                    _instance.enable_colors();
                }
                _instance.enable_unicode();
                _instance._state = State::MODIFIED;
            }

            /// @brief Hides/disables console window for the entire duration of this app run.
            static void hide_permanently() {
                _instance.disable_features();
                if (_instance._state != State::FREED) {
                    FreeConsole();
                    _instance._state = State::FREED;
                }
            }

        private:
            WindowsConsole() {}

            WindowsConsole(const WindowsConsole&) = delete;
            WindowsConsole& operator=(const WindowsConsole&) = delete;

            ~WindowsConsole() {
                disable_features();
            }

            /// @brief Disables all features.
            void disable_features() {
                if (_state == State::MODIFIED) {
                    disable_unicode();
                    if (should_show_colored_output()) {
                        disable_colors();
                    }
                    _state = State::DEFAULT;
                }
            }

            /// @brief Saves current console mode and enables virtual terminal processing (ANSI color codes).
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
    
    #else
        
        /// @brief Windows console encoding and ANSI codes setup class. No-op on other platforms.
        class WindowsConsole {
        public:
            /// @brief Enables all features.
            static void enable_features() {}
            /// @brief Hides/disables console window for the entire duration of this app run.
            static void hide_permanently() {}
        };

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

    /// @brief Initializes config directory and logging.
    /// On Windows also initializes 'date' library.
    /// @throws usos_rpc::Exception when file operations fail.
    void initialize_config() {  // clang-format off
        try {
            assert_files();
        } catch (const std::filesystem::filesystem_error& err) {
            throw Exception(ExceptionType::IO, err.code().message());
        }

        #ifdef _WIN32
            auto tzdata_path = *get_config_directory() / "tzdata";
            date::set_install(tzdata_path.string());
        #endif

        initialize_logging();
    }  // clang-format on
}
