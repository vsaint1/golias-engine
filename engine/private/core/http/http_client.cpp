#include "core/http/http_client.h"

#include "core/engine.h"

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* s = static_cast<std::string*>(userdata);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

#if defined(SDL_PLATFORM_EMSCRIPTEN)
#include <emscripten/fetch.h>
#endif


HttpClient::HttpClient() {

}

HttpClient::~HttpClient() {

}

HttpResponse HttpClient::request(const HttpRequest& request) const {
    HttpResponse res;

#if defined(SDL_PLATFORM_EMSCRIPTEN)
    LOG_ERROR("Sync. HTTP/s not supported on Web builds.");
#else
     CURL* _curl = curl_easy_init();
    if (_curl) {
        curl_easy_setopt(_curl, CURLOPT_URL, request.url);

        if (request.method == "GET") {
            curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);
        } else if (request.method == "POST") {
            curl_easy_setopt(_curl, CURLOPT_POST, 1L);
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, request.body.c_str());
        } else {
            curl_easy_setopt(_curl, CURLOPT_CUSTOMREQUEST, request.method.c_str());
            if (!request.body.empty()) {
                curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, request.body.c_str());
            }
        }

        curl_slist* headers = nullptr;
        for (auto& h : request.headers) {
            headers = curl_slist_append(headers, (h.first + ": " + h.second).c_str());
        }

        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, headers);

        std::string response_data;
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response_data);

        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);

        CURLcode code = curl_easy_perform(_curl);
        if (code == CURLE_OK) {
            long status = 0;
            curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &status);
            res.status_code = static_cast<int>(status);
            res.body        = response_data;
        } else {
            LOG_ERROR("HttpRequest Error %s", curl_easy_strerror(code));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(_curl);
    }
#endif
    return res;
}

void HttpClient::request_async(HttpRequest request, const std::function<void(const HttpResponse&)>& callback) const {
#if defined(SDL_PLATFORM_EMSCRIPTEN)
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, request.method.c_str());

    if (!request.body.empty()) {
        attr.requestData     = request.body.c_str();
        attr.requestDataSize = request.body.size();
    }

    std::vector<const char*> headers;
    for (auto& h : request.headers) {
        std::string kv = h.first + ": " + h.second;
        headers.push_back(strdup(kv.c_str()));
    }
    headers.push_back(nullptr);
    attr.requestHeaders = headers.data();

    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    auto* cbPtr   = new std::function<void(HttpResponse)>(callback);
    attr.userData = cbPtr;

    attr.onsuccess = +[](emscripten_fetch_t* fetch) {
        HttpResponse res;
        res.status_code = fetch->status;
        res.body.assign(fetch->data, fetch->numBytes);

        auto cb = (std::function<void(HttpResponse)>*) fetch->userData;
        (*cb)(res);
        delete cb;

        emscripten_fetch_close(fetch);
    };

    attr.onerror = +[](emscripten_fetch_t* fetch) {
        LOG_ERROR("HttpRequest failed (emscripten)");
        HttpResponse res;
        res.status_code = fetch->status;

        auto cb = (std::function<void(HttpResponse)>*) fetch->userData;
        (*cb)(res);
        delete cb;

        emscripten_fetch_close(fetch);
    };

    emscripten_fetch(&attr, request.url);

    for (auto h : headers) {
        free((void*) h);
    }

#else
    const auto fut = GEngine->get_system<ThreadManager>()->enqueue([this, request = std::move(request), callback = std::move(callback)] {
        try {
            HttpResponse res = this->request(request);
            callback(std::move(res));
        } catch (const std::exception& e) {
            LOG_ERROR("HttpRequest async exception: %s", e.what());
        }
    });

    (void) fut;
#endif
}
