/// @file
/// @brief Linux service manipulation commands via systemd.

#pragma once

#include <cstdlib>
#include <filesystem>

#include "../files.hpp"
#include "../logging.hpp"

#include "battery/embed.hpp"

namespace usos_rpc::commands {

    /// @brief Creates or updates systemd service.
    /// @throws usos_rpc::Exception when system commands or file operations fail
    void install() {
        auto service_path = get_base_directory() / "systemd" / "user";
        std::filesystem::create_directories(service_path);
        service_path /= "usos-rpc.service";
        auto contents = fmt::format(
            fmt::runtime(b::embed<"resources/usos-rpc.service">().data()),
            fmt::arg("exec", get_executable_path().c_str())
        );
        write_file(service_path.string(), contents);

        auto status = std::system("systemctl --user daemon-reload");
        if (status != 0) {
            throw Exception(ExceptionType::SYSTEMD, "Failed to reload user services!");
        }
        status = std::system("systemctl --user enable usos-rpc.service");
        if (status != 0) {
            throw Exception(ExceptionType::SYSTEMD, "Failed to enable service!");
        }
        lprint(colors::SUCCESS, "Service enabled successfully!\n");
    }

    /// @brief Deletes systemd service.
    /// @throws usos_rpc::Exception when system commands or file operations fail
    void uninstall() {
        auto status = std::system("systemctl --user disable usos-rpc.service");
        if (status != 0) {
            throw Exception(ExceptionType::SYSTEMD, "Failed to disable service!");
        }
        auto service_path = get_base_directory() / "systemd" / "user" / "usos-rpc.service";
        std::filesystem::remove(service_path);
        lprint(colors::SUCCESS, "Service disabled successfully!\n");
    }

}
