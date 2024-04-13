#pragma once

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>

#include "exceptions.hpp"

// clang-format off
#ifndef USOS_RPC_STD_ONLY

    #ifdef _WIN32
        #include "shlobj_core.h"
    #else
        #include <unistd.h>
        #include <pwd.h>
    #endif

    namespace {

        /// @brief Returns system-specific home directory.
        /// @return %AppData% on Windows, $HOME on Unix
        [[nodiscard]]
        std::filesystem::path get_home_directory() {
            #ifdef _WIN32
                PWSTR result = nullptr;
                SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &result);
                std::filesystem::path dir(result);
                CoTaskMemFree(result);
                return dir;
            #else
                const char* home = std::getenv("HOME");
                if (home == nullptr) {
                    home = getpwuid(getuid())->pw_dir;
                }
                return std::filesystem::path(home);
            #endif
        }

    }

#endif
// clang-format on

namespace usos_rpc {

    /// @brief Returns config directory path based on the value of environmental variable USOS_RPC_DIR
    /// or user's home directory. The path is cached after the first call.
    /// @return guaranteed-to-be-valid directory path
    // clang-format off
    [[nodiscard]]
    const std::filesystem::path* const get_config_directory() {
        using namespace std::filesystem;
        static std::unique_ptr<path> cache = nullptr;
        if (cache) {
            return cache.get();
        }

        // environment variable
        auto env_dir = std::getenv("USOS_RPC_DIR");
        if (env_dir) {
            path env_path(env_dir);
            if (is_directory(env_path)) {
                cache = std::make_unique<path>(env_path);
                return cache.get();
            }
        }

        // home directory or AppData
        #ifndef USOS_RPC_STD_ONLY
            path home = get_home_directory();
            if (is_directory(home)) {
                home /= "usos-rpc";
                if (!exists(home)) {
                    create_directory(home);
                }
                if (is_directory(home)) {
                    cache = std::make_unique<path>(home);
                    return cache.get();
                }
            }
        #endif

        // last hope
        cache = std::make_unique<path>(current_path());
        return cache.get();
    }

    // clang-format on

    /// @brief Reads file contents.
    /// @param path file path to read
    /// @return file contents
    std::string read_file(const std::string& path) {
        std::ifstream reader(path);
        if (!reader) {
            throw Exception(ExceptionType::IO_ERROR, "Cannot read file contents ({})!", path);
        }
        reader.exceptions(std::ios_base::badbit);
        
        try {
            return std::string(std::istreambuf_iterator<char>(reader), std::istreambuf_iterator<char>());
        } catch (const std::ios_base::failure& err) {
            throw Exception(ExceptionType::IO_ERROR, "Cannot read file contents ({})!", path);
        }
    }

}
