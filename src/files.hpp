/// @file
/// @brief Functions for file operations and config directory paths.

#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>

#include "exceptions.hpp"

#ifdef _WIN32
    #include <shlobj.h>
#else
    #include <pwd.h>
    #include <unistd.h>
#endif

namespace {

    /// @brief Returns system-specific home directory.
    /// @return %AppData% on Windows, $HOME on Unix
    [[nodiscard]]
    std::filesystem::path get_home_directory() {  // clang-format off
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
    }  // clang-format on

}

namespace usos_rpc {

    /// @brief Returns config directory path based on the value of environmental variable USOS_RPC_DIR
    /// or user's home directory. The path is cached after the first call.
    /// @return guaranteed-to-be-valid directory path
    [[nodiscard]]
    const std::filesystem::path* const get_config_directory() {
        using namespace std::filesystem;
        static std::unique_ptr<path> cache = nullptr;
        if (cache) {
            return cache.get();
        }

        try {
            // environment variable
            auto env_dir = std::getenv("USOS_RPC_DIR");
            if (env_dir) {
                path env_path(env_dir);
                if (!exists(env_path)) {
                    create_directories(env_path);
                }
                if (is_directory(env_path)) {
                    cache = std::make_unique<path>(env_path);
                    return cache.get();
                }
            }

            // home directory or AppData
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

            // last hope
            cache = std::make_unique<path>(current_path());
            return cache.get();
        } catch (const filesystem_error& fs_error) {
            throw Exception(ExceptionType::IO, fs_error.what());
        }
    }

    /// @brief Reads file contents.
    /// @param path file path to read
    /// @return file contents
    /// @throws usos_rpc::Exception when reading the file fails
    std::string read_file(const std::string& path) {
        std::ifstream reader(path, std::ios::binary);
        if (!reader) {
            throw Exception(ExceptionType::IO, "Cannot read file contents ({})!", path);
        }
        reader.exceptions(std::ios_base::badbit);

        try {
            return std::string(std::istreambuf_iterator<char>(reader), std::istreambuf_iterator<char>());
        } catch (const std::ios_base::failure& err) {
            throw Exception(ExceptionType::IO, "Cannot read file contents ({})!", path);
        }
    }

    /// @brief Writes file contents.
    /// @tparam T contents' type
    /// @param path file path to write
    /// @param contents contents to write
    /// @throws usos_rpc::Exception when writing the file fails
    template <typename T>
    void write_file(const std::string& path, const T& contents) {
        std::ofstream writer(path, std::ios::binary);
        if (!writer) {
            throw Exception(ExceptionType::IO, "Cannot write to file ({})!", path);
        }
        writer << contents;
    }
}
