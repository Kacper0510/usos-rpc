/// @file
/// @brief Windows-specific registry manipulation commands.

#pragma once

#include "../files.hpp"

#include <windows.h>

namespace {
    /// @brief Autostart registry key.
    constexpr const wchar_t* AUTOSTART_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
}

namespace usos_rpc::commands {

    /// @brief Creates or modifies autostart entry.
    /// @throws usos_rpc::Exception when registry operations fail
    void install() {
        auto exe_path = fmt::format(L"\"{}\" -b", get_executable_path().native());
        auto status = RegSetKeyValueW(
            HKEY_CURRENT_USER,
            AUTOSTART_KEY,
            L"usos-rpc",
            REG_SZ,
            exe_path.c_str(),
            (std::wcslen(exe_path.c_str()) + 1) * sizeof(wchar_t)
        );
        if (status != ERROR_SUCCESS) {
            LPTSTR error_message = nullptr;
            FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                status,
                0,
                (LPTSTR) &error_message,
                0,
                nullptr
            );
            Exception ex(ExceptionType::REGISTRY, error_message);
            LocalFree(error_message);
            throw ex;
        }
        lprint(colors::SUCCESS, "Autostart enabled successfully!\n");
    }

    /// @brief Deletes autostart entry.
    /// @throws usos_rpc::Exception when registry operations fail
    void uninstall() {
        HKEY key_path = nullptr;
        auto status = RegOpenKeyExW(HKEY_CURRENT_USER, AUTOSTART_KEY, 0, KEY_SET_VALUE, &key_path);
        if (status != ERROR_SUCCESS) {
            goto handle_error;
        }
        status = RegDeleteValueW(key_path, L"usos-rpc");
        if (status != ERROR_SUCCESS) {
            goto handle_error;
        }
        status = RegCloseKey(key_path);
        if (status != ERROR_SUCCESS) {
            goto handle_error;
        }
        lprint(colors::SUCCESS, "Autostart disabled successfully!\n");
        return;

    handle_error:
        LPTSTR error_message = nullptr;
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            status,
            0,
            (LPTSTR) &error_message,
            0,
            nullptr
        );
        Exception ex(ExceptionType::REGISTRY, error_message);
        LocalFree(error_message);
        throw ex;
    }

}
