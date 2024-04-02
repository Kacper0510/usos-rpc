#pragma once

#include <filesystem>
#include <cstdio>
#include <memory>

#ifdef _WIN32
    #include "shlobj_core.h"
#else

#endif

namespace {

    /// @brief Returns system-specific home directory.
    /// @return %AppData% on Windows, $HOME on Unix
    std::filesystem::path get_home_directory() {
        #ifdef _WIN32
            PWSTR result = nullptr;
            SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &result);
            std::filesystem::path dir(result);
            CoTaskMemFree(result);
            return dir;
        #else
            // TODO
        #endif
    }

}

namespace usos_rpc {

    /// @brief Returns config directory path based on the value of environmental variable USOS_RPC_DIR or user's home directory. 
    /// The path is cached after the first call.
    /// @return guaranteed-to-be-valid directory path
    const std::filesystem::path* get_config_directory() {
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
    }

}