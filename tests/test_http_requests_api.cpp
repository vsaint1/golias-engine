#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int WINDOW_WIDTH  = 1280;
int WINDOW_HEIGHT = 720;

int main(int argc, char* argv[]) {

    if (!GEngine->initialize(WINDOW_WIDTH, WINDOW_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return SDL_APP_FAILURE;
    }

    HttpRequest request_get("https://jsonplaceholder.typicode.com/todos/1");
    HttpClient client;

    HttpRequest request_post("https://jsonplaceholder.typicode.com/posts", "POST");
    request_post.headers["Content-Type"] = "application/json; charset=UTF-8";
    request_post.body                    = R"({"title": "foo", "body": "bar", "userId": 1})";

    EMBER_TIMER_START();

    client.request_async(request_post, [](const HttpResponse& res) { LOG_INFO("Async POST response \n%s", res.body.c_str()); });
    EMBER_TIMER_END("Request POST Async");


    client.request_async(request_get, [](const HttpResponse& res) { LOG_INFO("Async response %s", res.body.c_str()); });

    EMBER_TIMER_END("Request GET Async");

    auto response = client.request(request_get);
    LOG_INFO("Sync response %s", response.body.c_str());

    EMBER_TIMER_END("Request GET Sync");

    // SDL_Event e;
    // while (GEngine->is_running) {
    //     while (SDL_PollEvent(&e)) {
    //         if (e.type == SDL_EVENT_QUIT) {
    //             GEngine->is_running = false;
    //         }
    //     }
    // }

    SDL_Delay(3000);

    GEngine->shutdown();

    return 0;
}
