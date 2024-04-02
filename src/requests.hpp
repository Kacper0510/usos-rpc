#pragma once

#include <string>

#include "version_info.hpp"

#include "curl/curl.h"
#include "fmt/core.h"

namespace {

    /// @brief Internal callback for libcurl's CURLOPT_WRITEFUNCTION.
    /// @see https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
    /// @param buffer new data to handle
    /// @param size sizeof(char)
    /// @param nmemb length of the buffer
    /// @param userp pointer to user data, in this case - std::string
    /// @return the value of nmemb, signifying no error
    std::size_t _libcurl_callback(const char *buffer, std::size_t size, std::size_t nmemb, void *userp) {
        size *= nmemb;

        auto str = (std::string*) userp;
        str->append(buffer, size);

        return size;
    }

}

namespace usos_rpc {

    /// @brief Performs a simple HTTP GET request.
    /// @param url URL of the request
    /// @return response data
    std::string http_get(const char* url) {
        auto handle = curl_easy_init();
        if (!handle) {
            throw std::string("Failed to initialize Curl!");
        }

        std::string response;
        curl_easy_setopt(handle, CURLOPT_URL, url);
        curl_easy_setopt(handle, CURLOPT_USERAGENT, USER_AGENT);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, _libcurl_callback);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
        auto success = curl_easy_perform(handle);

        curl_easy_cleanup(handle);
        if (success != 0) {
            throw fmt::format("Request failed: {}", curl_easy_strerror(success));
        }
        return response;
    }

}
