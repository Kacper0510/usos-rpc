#pragma once

#include <string>
#include <string_view>

#include "build_info.hpp"
#include "exceptions.hpp"
#include "files.hpp"
#include "utilities.hpp"

#include "curl/curl.h"

namespace {

    /// @brief Internal callback for libcurl's CURLOPT_WRITEFUNCTION.
    /// @see https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
    /// @param buffer new data to handle
    /// @param size sizeof(char)
    /// @param nmemb length of the buffer
    /// @param userp pointer to user data, in this case - std::string
    /// @return the value of nmemb, signifying no error
    std::size_t libcurl_callback(const char* buffer, std::size_t size, std::size_t nmemb, void* userp) {
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
    /// @throws Exception when the request or libcurl fails
    std::string http_get(const char* url) {
        auto handle = curl_easy_init();
        if (!handle) {
            throw Exception(ExceptionType::CURL_ERROR, "Failed to initialize Curl!");
        }

        std::string response;
        curl_easy_setopt(handle, CURLOPT_URL, url);
        curl_easy_setopt(handle, CURLOPT_USERAGENT, USER_AGENT);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, libcurl_callback);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, true);
        auto success = curl_easy_perform(handle);

        curl_easy_cleanup(handle);
        if (success != 0) {
            throw Exception(ExceptionType::CURL_ERROR, "Request failed: {}", curl_easy_strerror(success));
        }
        return response;
    }

    /// @brief Fetches contents from a website or from a file (type of data detected automatically).
    /// If the URI protocol is webcal(s), it is translated to http(s) accordingly.
    /// @param path URI or file path
    /// @return response data
    /// @throws Exception when the underlying function calls fail
    std::string fetch_content(const std::string& path) {
        if (path.starts_with("webcal://")) {
            std::string_view real_path(path.begin() + const_string_length("webcal://"), path.end());
            std::string new_path("http://");
            new_path.append(real_path);
            return http_get(new_path.c_str());
        } else if (path.starts_with("webcals://")) {
            std::string_view real_path(path.begin() + const_string_length("webcals://"), path.end());
            std::string new_path("https://");
            new_path.append(real_path);
            return http_get(new_path.c_str());
        } else if (path.starts_with("http://") || path.starts_with("https://")) {
            return http_get(path.c_str());
        } else {
            return read_file(path);
        }
    }

}
