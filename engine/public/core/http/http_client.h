#pragma once
#include "core/systems/logging_sys.h"
#include <curl/curl.h>

struct HttpRequest {
    const char* url    = ""; ///< URL to send the request to.
    std::string method = "GET"; ///< HTTP method (GET, POST, PUT, PATCH, DELETE).
    std::map<std::string, std::string> headers;
    std::string body; ///< Request body for POST/PUT requests.

    explicit HttpRequest(const char* url = "", const std::string& method = "GET") : url(url), method(method) {}
};

struct HttpResponse {
    int status_code = 0; ///< HTTP status code.
    std::string body; ///< Response body.
    std::map<std::string, std::string> headers; ///< Response headers.
};

/*!
 * @brief Simple HTTP client
 *
 *
 * @details
 * - Uses libcurl for HTTP requests (not supported on Emscripten/WebAssembly).
 * - Supports GET, POST, PUT, DELETE methods.
 * - Allows setting custom headers and request body.
 * - Provides both blocking and non-blocking (async) request methods.
 *
 * @version 1.2.0
 *
 * @note
 *  - Synchronous requests are not supported on Emscripten.
 *
 *  @return HttpResponse
 *
 */
class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    /// Blocking (not supported on Emscripten)
    [[nodiscard]] HttpResponse request(const HttpRequest& request) const;

    /// Non-blocking (cross-platform)
    void request_async(HttpRequest request, const std::function<void(const HttpResponse&)>& callback) const;

};
