#include <string>

#include "files.hpp"
#include "requests.hpp"
#include "version_info.hpp"

#include "curl/curl.h"
#include "fmt/core.h"

/// @brief Entry point of the program.
/// @return process return code
int main() {
    // this function currently only tests some features

    fmt::println("USOS Rich Presence v{}", usos_rpc::VERSION);
    fmt::println("Curl: {}", curl_version());
    fmt::println("User agent: {}", usos_rpc::USER_AGENT);

    try {
        auto response = usos_rpc::http_get("https://api.ipify.org/");
        fmt::println("IP: {}", response);
    } catch (std::string err) {
        fmt::println(stderr, "Error:\n{}", err);
    }

    fmt::println("Home path: {}", usos_rpc::get_config_directory()->string());

    return 0;
}
