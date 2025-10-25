#include "core/engine.h"

std::unique_ptr<Engine> GEngine = std::make_unique<Engine>();

Renderer* create_renderer_internal(SDL_Window* window, EngineConfig& config) {


    Renderer* renderer = nullptr;
    switch (config.get_renderer_device().backend) {
    case Backend::GL_COMPATIBILITY: {
        renderer = new OpenGLRenderer();
        break;
    }
    case Backend::VK_FORWARD:
        spdlog::error("Vulkan backend is not yet supported");
        break;
    case Backend::DIRECTX12:
        spdlog::error("DirectX 12 backend is not yet supported");
        break;
    case Backend::METAL:
        spdlog::error("Metal backend is not yet supported");
        break;
    case Backend::AUTO: {
        spdlog::error("SDL Renderer backend is not yet supported");
        break;
    }
    }

    // TODO: later use viewport
    const auto& viewport = config.get_window();

    if (!renderer || !renderer->initialize(viewport.width, viewport.height)) {
        spdlog::error("Renderer initialization failed, shutting down");

        delete renderer;

        return nullptr;
    }


    return renderer;
}


bool Engine::initialize(int window_w, int window_h, const char* title, Uint32 window_flags) {


#if defined(NDEBUG)
    const auto LOG_LEVEL = spdlog::level::info;
#else
    const auto LOG_LEVEL = spdlog::level::debug;
#endif

#if defined(__ANDROID__)
    auto android_sink = std::make_shared<spdlog::sinks::android_sink_mt>("GoliasEngine");
    std::vector<spdlog::sink_ptr> sinks{android_sink};
#else
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/output.log", true);
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
#endif

    auto logger = std::make_shared<spdlog::logger>("GoliasEngine", sinks.begin(), sinks.end());

#if defined(NDEBUG)
    logger->set_level(spdlog::level::info);
#else
    logger->set_level(spdlog::level::debug);
#endif

    if (!_config.load()) {
        spdlog::warn("Using default configuration values");
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

    spdlog::info("Initializing {}, Version {}", ENGINE_NAME, ENGINE_VERSION_STR);

    spdlog::info("Project Configuration -> Window: ({}x{}), ApplicationName: {}, Version: {}, Package: {}",
                 app_win.width, app_win.height,
                 app_config.name, app_config.version, app_config.package_name);

#pragma region APP_METADATA
    SDL_SetHint(SDL_HINT_ORIENTATIONS, _config.get_orientation_str());
    SDL_SetAppMetadata(app_config.name, app_config.version, app_config.package_name);
#pragma endregion


    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO)) {
        spdlog::error("Engine initialization failed: {}", SDL_GetError());
        return false;
    }


    if (!TTF_Init()) {
        spdlog::error("TTF_Init failed: {}", SDL_GetError());
        return false;
    }


    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");


    app_win.width  = window_w;
    app_win.height = window_h;

    int driver_count = SDL_GetNumRenderDrivers();

    if (driver_count < 1) {
        spdlog::critical("No render drivers available");
        return false;
    }

    std::string renderer_list;
    renderer_list.reserve(driver_count * 16);
    for (int i = 0; i < driver_count; ++i) {
        const char* name = SDL_GetRenderDriver(i);
        renderer_list += name;
        renderer_list += (i < driver_count - 1) ? ", " : "";
    }

    spdlog::info("Available Backends Count {}, Options {}", driver_count, renderer_list.c_str());


    _window = SDL_CreateWindow(app_config.name, window_w, window_h, window_flags);

    if (!_window) {
        spdlog::error("Window creation failed: {}", SDL_GetError());
        SDL_Quit();
        return false;
    }

#pragma region ENGINE_WINDOW_ICON

    int w, h, channels;
    SDL_Surface* logo_surface = nullptr;
    stbi_uc* logo_pixels      = stbi_load((ASSETS_PATH + "icon.png").c_str(), &w, &h, &channels, 4);

    if (logo_pixels) {
        logo_surface = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, logo_pixels, w * 4);
        SDL_SetWindowIcon(_window, logo_surface);
        SDL_DestroySurface(logo_surface);
        stbi_image_free(logo_pixels);
    } else {
        spdlog::error("Failed to Load default `icon` Image");
    }

#pragma endregion


    spdlog::info("Backend selected: {}", _config.get_renderer_device().get_backend_str());


    // TODO: later we can add support for other renderers (Vulkan, OpenGL, etc.)
    _renderer = create_renderer_internal(_window, _config);

    if (!_renderer) {
        spdlog::error("Renderer creation failed, shutting down");
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return false;
    }


    _timer.start();

    SDL_ShowWindow(_window); // now shown after renderer  setup

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


void engine_draw_loop() {
    std::vector<DirectionalLight> directionalLights;
    glm::mat4 lightSpaceMatrix(1.0f);

    Camera3D mainCamera;
    GEngine->get_world().each([&](const Camera3D& cam) {
        mainCamera = cam;
    });

    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, DirectionalLight& light) {
        directionalLights.push_back(light);

        if (light.castShadows && lightSpaceMatrix == glm::mat4(1.0f)) {
            lightSpaceMatrix = light.get_light_space_matrix();
        }
    });

    std::vector<std::pair<Transform3D, SpotLight>> spotLights;
    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, SpotLight& light) {
        spotLights.push_back({t, light});
    });

    // Shadow pass
    GEngine->get_renderer()->begin_shadow_pass();
    GEngine->get_world().each([&](Transform3D& t, MeshInstance3D& mesh, Material& mat) {
        GEngine->get_renderer()->render_shadow_pass(t, mesh, lightSpaceMatrix);
    });
    GEngine->get_renderer()->end_shadow_pass();

    // Main render pass
    GEngine->get_renderer()->begin_render_target();
    GEngine->get_world().each([&](Transform3D& t, MeshInstance3D& mesh, Material& mat) {
        GEngine->get_renderer()->render_entity(t, mesh, mat, mainCamera, lightSpaceMatrix,
                                               directionalLights, spotLights);
    });
    GEngine->get_renderer()->end_render_target();

    GEngine->get_renderer()->swap_chain();


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

    }

    // TODO: add script logic and remove this
    const bool* scancodes = SDL_GetKeyboardState(nullptr);

    GEngine->get_world().each([&](flecs::entity e, Transform3D& transform, Camera3D& camera) {

        float dt = static_cast<float>(GEngine->get_timer().delta);

        if (scancodes[SDL_SCANCODE_W]) {
            camera.move_forward(transform, dt);
        }
        if (scancodes[SDL_SCANCODE_S]) {
            camera.move_backward(transform, dt);
        }
        if (scancodes[SDL_SCANCODE_A]) {
            camera.move_left(transform, dt);
        }
        if (scancodes[SDL_SCANCODE_D]) {
            camera.move_right(transform, dt);
        }

        if (scancodes[SDL_SCANCODE_SPACE]) {
            transform.position.y += camera.speed * dt;
        }

        if (scancodes[SDL_SCANCODE_LCTRL]) {
            transform.position.y -= camera.speed * dt;
        }

        if (scancodes[SDL_SCANCODE_Q]) {
            camera.look_at(-200.0f * dt, 0.0f);
        }

        if (scancodes[SDL_SCANCODE_E]) {
            camera.look_at(200.0f * dt, 0.0f);
        }

        if (scancodes[SDL_SCANCODE_LSHIFT]) {
            camera.speed = 150.0f;
        }else {
            camera.speed = 50.0f;
        }

    });


    GEngine->get_world().progress(static_cast<float>(GEngine->get_timer().delta));

    engine_draw_loop();

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

    _renderer->cleanup();

    delete _renderer;

    SDL_DestroyWindow(_window);

    TTF_Quit();
    SDL_Quit();

}
