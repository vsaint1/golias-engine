#pragma once
#include "core/system/logging.h"
#include "core/system/timer.h"
#include "core/renderer/sdl_renderer.h"

class Engine {
public:
    bool initialize(int window_w, int window_h, const char* title = "Ember Engine - Window", Uint32 window_flags = SDL_WINDOW_RESIZABLE);

    void run();

    Timer& get_timer();

    Renderer* get_renderer() const;

    SDL_Window* get_window() const;

    flecs::world& get_world() { return _world; }

    bool is_running = false;

    SDL_Event event;

    ~Engine();
    Timer _timer;

private:
    flecs::world _world;
    SDL_Window* _window     = nullptr;
    Renderer* _renderer     = nullptr;
};

void engine_core_loop();

inline std::unique_ptr<Engine> GEngine = std::make_unique<Engine>();



