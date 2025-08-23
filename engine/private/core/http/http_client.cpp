#include "core/http/http_client.h"

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* s = (std::string*)userdata;
    s->append(ptr, size * nmemb);
    return size * nmemb;
}


HttpClient::HttpClient() {
    _curl = curl_easy_init();
}

HttpClient::~HttpClient() {
    curl_easy_cleanup(_curl);
    _curl = nullptr;
}

HttpResponse HttpClient::request(const HttpRequest& request) const {

    HttpResponse res;

    if (_curl) {
        curl_easy_setopt(_curl, CURLOPT_URL, request.url);

        if (request.method == "POST") {
            curl_easy_setopt(_curl, CURLOPT_POST, 1L);
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, request.body.c_str());
        }

        curl_slist* headers = nullptr;
        for (auto& h : request.headers) {
            headers = curl_slist_append(headers, (h.first + ": " + h.second).c_str());
        }

        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);

        std::string response_data;
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_callback);

        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response_data);

        CURLcode code = curl_easy_perform(_curl);
        if(code == CURLE_OK) {
            long status = 0;
            curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &status);
            res.status_code = static_cast<int>(status);
            res.body = response_data;
        }else {
            LOG_ERROR("HttpRequest Error %s", curl_easy_strerror(code));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(_curl);
    }


    return res;
}
