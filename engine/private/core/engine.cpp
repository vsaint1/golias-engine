#include "core/engine.h"

bool Engine::initialize(int window_w, int window_h, const char* title, Uint32 window_flags) {


    LOG_INFO("Initializing %s, Version %s", ENGINE_NAME, ENGINE_VERSION_STR);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO)) {
        LOG_ERROR("Engine initialization failed %s", SDL_GetError());
        return false;
    }


    if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        return false;
    }

    _window = SDL_CreateWindow(title, window_w, window_h, window_flags);

    if (!_window) {
        LOG_ERROR("Window creation failed %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

#pragma region ENGINE_WINDOW_ICON

    SDL_Surface* logo_surface = IMG_Load((ASSETS_PATH + "icon.png").c_str());

    if (logo_surface) {
        SDL_SetWindowIcon(_window, logo_surface);
        SDL_DestroySurface(logo_surface);
    } else {
        LOG_ERROR("Failed to load `icon` image: %s", SDL_GetError());
    }

#pragma endregion

    // TODO: later we can add support for other renderers (Vulkan, OpenGL, etc.)
    SDLRenderer* renderer = new SDLRenderer();

    if (!renderer->initialize(_window)) {
        LOG_ERROR("Renderer initialization failed");
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return false;
    }


    renderer->load_font("default", "res/fonts/Default.ttf", 16);
    renderer->load_font("emoji", "res/fonts/Twemoji.ttf", 16);
    renderer->set_default_fonts("default", "emoji");

#pragma region SETUP_FLECS_WORLD

    this->_world.component<Transform2D>();
    this->_world.component<Shape>();
    this->_world.component<Script>();
    this->_world.component<Label2D>();


    engine_setup_systems(this->_world);

    serialize_components(this->_world);

#pragma endregion

    _renderer = renderer;

    _timer.start();

    is_running = true;

    return true;
}

Timer& Engine::get_timer() {
    return _timer;
}


Renderer* Engine::get_renderer() const {
    return _renderer;
}

SDL_Window* Engine::get_window() const {
    return _window;
}

flecs::world& Engine::get_world() {
    return _world;
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

    delete _renderer;

    SDL_DestroyWindow(_window);


    TTF_Quit();
    SDL_Quit();
}

void engine_core_loop() {

    GEngine->get_timer().tick();

    while (SDL_PollEvent(&GEngine->event)) {
        if (GEngine->event.type == SDL_EVENT_QUIT) {
            GEngine->is_running = false;
        }
    }

    GEngine->get_renderer()->clear({0.2, 0.2, 0.2, 1.0f});

    GEngine->get_world().progress(static_cast<float>(GEngine->get_timer().delta));

    GEngine->get_renderer()->present();

    // FIXME: Cap framerate for now
    SDL_Delay(16); // ~60 FPS
}


void engine_setup_systems(flecs::world& world) {
    world.system<Transform2D, Shape>("RenderPrimitives_OnUpdate").kind(flecs::OnUpdate).each(render_primitives_system);

    world.system<Transform2D, Label2D>("RenderText_OnUpdate").kind(flecs::OnUpdate).each(render_labels_system);


    world.system<Script>("LoadScripts_OnStart").kind(flecs::OnStart).each([&](flecs::entity e, Script& s) {
      
        if (s.path.empty()) {
            LOG_WARN("Script component on entity %s has empty path", e.name().c_str());
            return;
        }

        load_scripts_system(s);


        generate_bindings_to_lua(s.lua_state, world, e);
    });

    world.system<Script>().kind(flecs::OnUpdate).each(process_scripts_system);
}
