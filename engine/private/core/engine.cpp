#include "core/engine.h"

#include "core/binding/lua.h"


bool Engine::initialize(int window_w, int window_h, const char* title, Uint32 window_flags) {


    if (!_config.load()) {
        LOG_CRITICAL("Failed to load config file (project.xml)");
    }

    const auto& app_config = _config.get_application();


    if (app_config.is_fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN;
    }

    if (app_config.is_resizable) {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }

    LOG_INFO("Initializing %s, Version %s", ENGINE_NAME, ENGINE_VERSION_STR);
    LOG_INFO("Application: %s, Version %s, Package: %s", app_config.name, app_config.version, app_config.package_name);


#pragma region APP_METADATA
    SDL_SetHint(SDL_HINT_ORIENTATIONS, _config.get_orientation_str());
    SDL_SetAppMetadata(app_config.name, app_config.version, app_config.package_name);
#pragma endregion


    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO)) {
        LOG_ERROR("Engine initialization failed %s", SDL_GetError());
        return false;
    }


    if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        return false;
    }

    auto& app_win = _config.get_window();

    app_win.width  = window_w;
    app_win.height = window_h;

    _window = SDL_CreateWindow(app_config.name, window_w, window_h, window_flags);

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

    if (_config.get_performance().is_multithreaded) {
        Logger::initialize();
    }


    LOG_INFO("Renderer Selected: %s", _config.get_renderer_device().get_backend_str());

    // TODO: later we can add support for other renderers (Vulkan, OpenGL, etc.)
    SDLRenderer* renderer = new SDLRenderer();

    if (!renderer->initialize(_window)) {
        LOG_ERROR("Renderer initialization failed, shutting down");

        delete renderer;

        SDL_DestroyWindow(_window);
        SDL_Quit();
        return false;
    }


    renderer->load_font("default", "res/fonts/Default.ttf", 16);
    renderer->load_font("emoji", "res/fonts/Twemoji.ttf", 16);
    renderer->set_default_fonts("default", "emoji");

#pragma region SETUP_FLECS_WORLD

    serialize_components(this->_world);

    engine_setup_systems(this->_world);


    LOG_INFO("Engine setup systems completed");

#pragma endregion

    _renderer = renderer;

    _renderer->load_texture("ui_icons", "res/ui/icons/icons_64.png");

    _timer.start();

    SDL_ShowWindow(_window);

    is_running = true;

    return true;
}

EngineConfig& Engine::get_config() {
    return _config;
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

        if(GEngine->event.type == SDL_EVENT_WINDOW_RESIZED) {
            int new_w = GEngine->event.window.data1;
            int new_h = GEngine->event.window.data2;
            auto& app_win = GEngine->get_config().get_window();
            app_win.width  = new_w;
            app_win.height = new_h;
        }
        
    }


    static flecs::entity selected;


    GEngine->get_renderer()->clear({0.2, 0.2, 0.2, 1.0f});

    GEngine->get_world().progress(static_cast<float>(GEngine->get_timer().delta));

    GEngine->get_renderer()->present();

    // FIXME: Cap framerate for now
    SDL_Delay(16); // ~60 FPS
}


void engine_setup_systems(flecs::world& world) {


    world.system<Transform2D>("UpdateTransforms_OnUpdate").kind(flecs::OnUpdate).with<ActiveScene>().up().each(update_transforms_system);

    world.system<Transform2D, Shape>("RenderPrimitives_OnUpdate")
        .kind(flecs::OnUpdate)
        .with<ActiveScene>()
        .up()
        .each([&](flecs::entity e, Transform2D& t, Shape& s) { render_primitives_system(t, s); });

    world.system<Transform2D, Label2D>("RenderText_OnUpdate")
        .kind(flecs::OnUpdate)
        .with<ActiveScene>()
        .up()
        .each([&](flecs::entity e, Transform2D& t, Label2D& l) { render_labels_system(t, l); });

    world.system<Transform2D, Sprite2D>("RenderSprite_OnUpdate")
        .kind(flecs::OnUpdate)
        .with<ActiveScene>()
        .up()
        .each([&](flecs::entity e, Transform2D& t, Sprite2D& s) { render_sprites_system(t, s); });

    scene_manager_system(world);

    world.system<Script>("LoadScripts_OnStart").kind(flecs::OnStart).with<ActiveScene>().up().each([&](flecs::entity e, Script& s) {
        if (s.path.empty()) {
            LOG_WARN("Script component on entity %s has empty path", e.name().c_str());
            return;
        }

        setup_scripts_system(e, s);
    });

    world.system<Script>("ProcessScripts_OnUpdate").kind(flecs::OnUpdate).with<ActiveScene>().up().each(process_scripts_system);
}
