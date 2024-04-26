#pragma once

#include <filesystem>

#include "exceptions.hpp"
#include "files.hpp"
#include "preinit_files.hpp"

#include "date/date.h"

namespace usos_rpc {

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
    /// On Windows also initializes 'date' library.
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
        #endif
    }  // clang-format on

}
