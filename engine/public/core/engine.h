#pragma once
#include "core/renderer/opengl/ogl_renderer.h"
#include "core/system/timer.h"
#include "core/utility/project_config.h"
#include "core/utility/obj_loader.h"

/*!
    @file engine.h
    @brief Engine class definition.

    This file contains the definition of the Engine class, which serves as the core of the Ember Engine. It manages the main loop, window creation, rendering, and event handling.

    @version 0.0.1

*/
class Engine {
public:
    bool initialize(int window_w, int window_h, const char* title = "Golias Engine - Window", Uint32 window_flags = SDL_WINDOW_RESIZABLE);

    void run();

    Timer& get_timer();

    Renderer* get_renderer() const;

    SDL_Window* get_window() const;

    EngineConfig& get_config();

    flecs::world& get_world();

    bool is_running = false;

    SDL_Event event;

    ~Engine();


private:
    EngineConfig _config = {};
    Timer _timer         = {};
    flecs::world _world;
    SDL_Window* _window = nullptr;
   Renderer* _renderer = nullptr;


};

/*!

    @brief Core engine loop function.

    @note This function should never be called directly. It is used internally by the engine to handle events, update the world, and render frames.

    @version 0.0.1
*/
void engine_core_loop();

void engine_draw_loop();

/*!

    @brief Sets up the core systems in the provided Flecs world.

    This function registers core systems required for the engine's operation

    @param world Reference to the Flecs world where systems will be registered.

    @version 0.0.1
*/

void engine_setup_systems(flecs::world& world);

extern std::unique_ptr<Engine> GEngine;