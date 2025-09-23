#pragma once
#include "core/system/logging.h"
#include "core/system/timer.h"


class Engine {
public:
    bool initialize(int window_w, int window_h, const char* title = "Ember Engine - Window", Uint32 window_flags = SDL_WINDOW_RESIZABLE);

    void run();

    Timer get_timer() const;

    SDL_Renderer* get_renderer() const;

    SDL_Window* get_window() const;

    bool is_running = false;

    SDL_Event event;

    ~Engine();

private:
    Timer _timer;
    SDL_Window* _window     = nullptr;
    SDL_Renderer* _renderer = nullptr;
};

void engine_core_loop();

inline std::unique_ptr<Engine> GEngine = std::make_unique<Engine>();
