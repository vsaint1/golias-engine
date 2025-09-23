#include "core/engine.h"

bool Engine::initialize(int window_w, int window_h, const char* title, Uint32 window_flags) {


    LOG_INFO("Initializing %s, Version %s", ENGINE_NAME, ENGINE_VERSION_STR);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO)) {
        LOG_ERROR("Engine initialization failed %s", SDL_GetError());
        return false;
    }

    _window = SDL_CreateWindow(title, window_w, window_h, window_flags);

    if (!_window) {
        LOG_ERROR("Window creation failed %s", SDL_GetError());
        SDL_Quit();
        return false;
    }


    int driver_count = SDL_GetNumRenderDrivers();


    std::string renderer_list;
    renderer_list.reserve(driver_count * 16);
    for (int i = 0; i < driver_count; ++i) {
        const char* name = SDL_GetRenderDriver(i);
        renderer_list += name;
        renderer_list += (i < driver_count - 1) ? ", " : "";
    }

    LOG_INFO("Available renderers (%d): %s", driver_count, renderer_list.c_str());


    _renderer = SDL_CreateRenderer(_window, nullptr);


    const char* renderer_name = SDL_GetRendererName(_renderer);

    _timer.start();

    is_running = true;

    LOG_INFO("Successfully initialized engine with %s renderer", renderer_name);

    return true;
}

Timer& Engine::get_timer()  {
    return _timer;
}

SDL_Renderer* Engine::get_renderer() const {
    return _renderer;
}

SDL_Window* Engine::get_window() const {
    return _window;
}

void Engine::run() {

#if defined(SDL_PLATFORM_EMSCRIPTEN)
    emscripten_set_main_loop(engine_core_loop, 60, 1);
#else
    while (is_running) {
        engine_core_loop();
    }

#endif
}


Engine::~Engine() {
    LOG_INFO("Shutting down engine");

    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}

void engine_core_loop() {

    GEngine->get_timer().tick();

    while (SDL_PollEvent(&GEngine->event)) {
        if (GEngine->event.type == SDL_EVENT_QUIT) {
            GEngine->is_running = false;
        }
    }

    SDL_SetRenderDrawColor(GEngine->get_renderer(), 20, 30, 50, 255);
    SDL_RenderClear(GEngine->get_renderer());

    GEngine->get_world().progress(static_cast<float>(GEngine->get_timer().delta));

    SDL_RenderPresent(GEngine->get_renderer());

    // FIXME: Cap framerate for now
    SDL_Delay(16); // ~60 FPS
}
