#pragma once
#include "helpers/logging.h"
#include <curl/curl.h>


struct HttpRequest {
    const char* url = ""; ///< URL to send the request to.
    std::string method = "GET"; ///< HTTP method (GET, POST, PUT, PATCH, DELETE).
    std::map<std::string, std::string> headers;
    std::string body; ///< Request body for POST/PUT requests.
};


struct HttpResponse {
    int status_code = 0; ///< HTTP status code.
    std::string body; ///< Response body.
    std::map<std::string, std::string> headers; ///< Response headers.
};


class HttpClient {
public:
    HttpClient();
    ~HttpClient();


    HttpResponse request(const HttpRequest& request) const;

private:
    CURL* _curl = nullptr;
};
