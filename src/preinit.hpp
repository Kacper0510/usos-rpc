#pragma once

#include <filesystem>

#include "files.hpp"
#include "preinit_files.hpp"

namespace usos_rpc {

    /// @brief Asserts that all necessary files exist in the config folder.
    /// Otherwise, it creates the files from cache embedded in the executable.
    void assert_files() {
        auto config_dir = *get_config_directory();
        for (const auto& [name, data] : EMBEDDED_FILES) {
            auto file = config_dir / name;
            if (!std::filesystem::exists(file)) {
                write_file(file.string(), data);
            }
        }
    }

}
