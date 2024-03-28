#include "version_info.hpp"
#include "fmt/core.h"
#include "curl/curl.h"

int main() {
    fmt::println("USOS Rich Presence v{}", USOS_RPC_VERSION);
    fmt::println("{}", curl_version());

    return 0;
}