#include "core/engine.h"

#include "core/binding/lua.h"

std::unique_ptr<Engine> GEngine = std::make_unique<Engine>();


Renderer* create_renderer_internal(SDL_Window* window, EngineConfig& config) {


    Renderer* renderer = nullptr;
    switch (config.get_renderer_device().backend) {
    case Backend::GL_COMPATIBILITY:
        {
            renderer = new OpenglRenderer();
            break;
        }
    case Backend::VK_FORWARD:
        LOG_ERROR("Vulkan backend is not yet supported");
        break;
    case Backend::DIRECTX12:
        LOG_ERROR("DirectX 12 backend is not yet supported");
        break;
    case Backend::METAL:
        LOG_ERROR("Metal backend is not yet supported");
        break;
    case Backend::AUTO:
        {
            renderer = new SDLRenderer();
            break;
        }
    }

    if (!renderer || !renderer->initialize(window)) {
        LOG_ERROR("Renderer initialization failed, shutting down");

        delete renderer;

        return nullptr;
    }


    return renderer;
}


bool Engine::initialize(int window_w, int window_h, const char* title, Uint32 window_flags) {

    // NOTE: set log only in debug mode
#if defined(NDEBUG)
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
#endif

    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);


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

    const auto& renderer_config = _config.get_renderer_device();

    if (renderer_config.backend == Backend::GL_COMPATIBILITY) {
        window_flags |= SDL_WINDOW_OPENGL;


        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }

    auto& app_win = _config.get_window();

    LOG_INFO("Initializing %s, Version %s", ENGINE_NAME, ENGINE_VERSION_STR);
    LOG_INFO("Project Configuration -> Window: (%dx%d), ApplicationName: %s, Version %s, Package: %s", app_win.width, app_win.height,
             app_config.name, app_config.version, app_config.package_name);


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


    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");



    app_win.width  = window_w;
    app_win.height = window_h;

    int driver_count = SDL_GetNumRenderDrivers();

    if (driver_count < 1) {
        LOG_ERROR("No render drivers available");
        return false;
    }

    std::string renderer_list;
    renderer_list.reserve(driver_count * 16);
    for (int i = 0; i < driver_count; ++i) {
        const char* name = SDL_GetRenderDriver(i);
        renderer_list += name;
        renderer_list += (i < driver_count - 1) ? ", " : "";
    }

    LOG_INFO("Available Backends Count (%d), List %s", driver_count, renderer_list.c_str());


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


    LOG_INFO("Backend Selected: %s", _config.get_renderer_device().get_backend_str());


    // TODO: later we can add support for other renderers (Vulkan, OpenGL, etc.)
    _renderer = create_renderer_internal(_window, _config);

    if (!_renderer) {
        LOG_ERROR("Renderer creation failed, shutting down");
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return false;
    }


    _renderer->load_font("default", "res/fonts/Default.ttf", 16);
    _renderer->load_font("emoji", "res/fonts/Twemoji.ttf", 16);
    _renderer->set_default_fonts("default", "emoji");

#pragma region SETUP_FLECS_WORLD

    serialize_components(this->_world);

    engine_setup_systems(this->_world);


    LOG_DEBUG("Engine setup systems completed");

#pragma endregion


    // _renderer->load_texture("ui_icons","res://ui/icons/icons_64.png"); // not used anymore -> nuklear

    _timer.start();

    // SDL_ShowWindow(_window); // now shown after renderer  setup

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

void engine_core_loop() {

    GEngine->get_timer().tick();


    while (SDL_PollEvent(&GEngine->event)) {

        if (GEngine->event.type == SDL_EVENT_QUIT) {
            GEngine->is_running = false;
        }


        if (GEngine->event.type == SDL_EVENT_KEY_DOWN) {

            if (GEngine->event.key.scancode == SDL_SCANCODE_F9) {
                GEngine->get_config().is_debug = !GEngine->get_config().is_debug;
            }
        }


        if (GEngine->event.type == SDL_EVENT_WINDOW_RESIZED) {
            int new_w      = GEngine->event.window.data1;
            int new_h      = GEngine->event.window.data2;
            auto& app_win  = GEngine->get_config().get_window();
            app_win.width  = new_w;
            app_win.height = new_h;
        }

        GEngine->get_world().each([&](flecs::entity e, const Script& script) { process_event_scripts_system(script, GEngine->event); });
    }


    GEngine->get_renderer()->clear(GEngine->get_config().get_environment().clear_color);

    GEngine->get_world().progress(static_cast<float>(GEngine->get_timer().delta));

    GEngine->get_renderer()->present();

    // FIXME: Cap framerate for now
    SDL_Delay(16); // ~60 FPS
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

void engine_setup_systems(flecs::world& world) {

#pragma region 3D SYSTEMS

    world.system<Model, Animation3D, Transform3D>("Animation_System_OnUpdate")
        .kind(flecs::OnUpdate)
        .with<tags::ActiveScene>()
        .up()
        .each(animation_system);

    world.system<Camera3D>("Render_World_3D_OnUpdate").kind(flecs::OnUpdate).with<tags::ActiveScene>().up().each(render_world_3d_system);

#pragma endregion


#pragma region 2D SYSTEMS
    world.system<Camera2D>("Render_World_2D_OnUpdate").kind(flecs::OnUpdate).each(render_world_2d_system);

#pragma endregion

    scene_manager_system(world);

    world.system<Script>("LoadScripts_OnStart").kind(flecs::OnStart).with<tags::ActiveScene>().up().each([&](flecs::entity e, Script& s) {
        if (s.path.empty()) {
            LOG_WARN("Script component on entity %s has empty path", e.name().c_str());
            return;
        }

        setup_scripts_system(e, s);
    });

    world.system<Script>("ProcessScripts_OnUpdate").kind(flecs::OnUpdate).with<tags::ActiveScene>().up().each(process_scripts_system);
}
