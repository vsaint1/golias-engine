#include "core/engine.h"

#include "core/component/comp_system.h"
#include "core/renderer/opengl/ogl_renderer.h"

std::unique_ptr<Engine> GEngine = std::make_unique<Engine>();

nk_context* nk_ctx = nullptr;

Renderer* create_renderer_internal(SDL_Window* window, EngineConfig& config) {


    Renderer* renderer = nullptr;
    switch (config.get_renderer_device().backend) {
    case Backend::GL_COMPATIBILITY:
        {
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
    case Backend::AUTO:
        {
            spdlog::error("SDL Renderer backend is not yet supported");
            break;
        }
    }

    const auto& win_size = config.get_window();

    if (!renderer || !renderer->initialize(win_size.width, win_size.height, window)) {
        spdlog::error("Renderer initialization failed, shutting down");

        delete renderer;

        return nullptr;
    }

    renderer->load_font("res/fonts/Default.ttf", 24, "default");
    renderer->load_font("res/fonts/Twemoji.ttf", 24, "emoji");

    return renderer;
}

void register_components(flecs::world& world) {
    // 2D Components
    world.component<Transform2D>();
    world.component<Camera2D>();
    world.component<Sprite2D>();
    world.component<Label2D>();
    world.component<Shape2D>();
    world.component<Follow>();
    world.component<SpriteRenderer2D>();

    // 3D Components
    world.component<Transform3D>();
    world.component<Camera3D>();
    world.component<DirectionalLight3D>();
    world.component<SpotLight3D>();
    world.component<MeshInstance3D>();
    world.component<WorldEnvironment3D>();

    // Common Components
    world.component<Tag>();
    world.component<NativeScript>();
    world.component<LuaScript>();
    world.component<WorldTransform>();

    world.component<tags::Scene>();
    world.component<tags::ActiveScene>();
    world.component<tags::MainCamera>();
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

    logger->set_level(LOG_LEVEL);
    logger->flush_on(LOG_LEVEL);

    spdlog::set_default_logger(logger);

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

    spdlog::info("Project Configuration -> Window: ({}x{}), ApplicationName: {}, Version: {}, Package: {}", app_win.width, app_win.height,
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

    nk_ctx = nk_sdl_init(_window);

    _timer.start();

    nk_font_atlas* atlas;
    // struct nk_font* font;


    nk_sdl_font_stash_begin(&atlas);
    // font = nk_font_atlas_add_default(atlas,14,0);
    nk_sdl_font_stash_end();

    register_components(_world);

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
#pragma region 3D_RENDERING_LOOP
   render_world_3d_system();
#pragma endregion

#pragma region 2D_RENDERING_LOOP
render_world_2d_system();
#pragma endregion

#pragma region UI_RENDERING_LOOP
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
#pragma endregion
    GEngine->get_renderer()->swap_chain();
}


nk_bool show_render_targets = false;



void draw_debug_ui() {

    if (nk_begin(nk_ctx, "Debug", nk_rect(10, 10, 400, 600), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(nk_ctx, 25, 1);
        nk_label(nk_ctx, "System Monitor", NK_TEXT_CENTERED);

        if (nk_tree_push(nk_ctx, NK_TREE_NODE, "GPU", NK_MINIMIZED)) {
            nk_layout_row_dynamic(nk_ctx, 20, 2);

            // TODO: Add your GPU data variables here
            static float gpu_usage     = 0.0f;
            static float gpu_temp      = 0.0f;
            static float gpu_clock     = 0.0f;
            static float gpu_mem_used  = 0.0f;
            static float gpu_mem_total = 0.0f;

            nk_labelf(nk_ctx, NK_TEXT_LEFT, "Device: %s", "GPU_Name_Placeholder");

            nk_layout_row_dynamic(nk_ctx, 2, 1);
            nk_spacing(nk_ctx, 1);

            // GPU Memory
            nk_layout_row_dynamic(nk_ctx, 18, 1);
            nk_label(nk_ctx, "VRAM Usage:", NK_TEXT_LEFT);

            nk_layout_row_dynamic(nk_ctx, 20, 2);
            nk_labelf(nk_ctx, NK_TEXT_LEFT, "%.2f MB / %.2f MB", gpu_mem_used, gpu_mem_total);

            nk_labelf(nk_ctx, NK_TEXT_LEFT, "%.1f%%", (gpu_mem_total > 0.0f) ? (gpu_mem_used / gpu_mem_total * 100.0f) : 0.0f);

            // Progress bar for VRAM
            nk_layout_row_dynamic(nk_ctx, 20, 1);
            nk_size vram_ratio = (gpu_mem_total > 0.0f) ? (gpu_mem_used / gpu_mem_total) : 0.0f;
            nk_progress(nk_ctx, &vram_ratio, 100, NK_FIXED);


            nk_tree_pop(nk_ctx);
        }


        // System Memory
        if (nk_tree_push(nk_ctx, NK_TREE_NODE, "System Memory", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(nk_ctx, 20, 2);

            // TODO: Add your memory data variables here
            static float ram_used      = 0.0f;
            static float ram_total     = 0.0f;
            static float ram_available = 0.0f;


            nk_label(nk_ctx, "Used:", NK_TEXT_LEFT);
            nk_labelf(nk_ctx, NK_TEXT_LEFT, "%.2f MB", ram_used);

            nk_label(nk_ctx, "Available:", NK_TEXT_LEFT);
            nk_labelf(nk_ctx, NK_TEXT_LEFT, "%.2f MB", ram_available);

            // Progress bar for RAM
            nk_layout_row_dynamic(nk_ctx, 20, 1);
            nk_size ram_ratio = (ram_total > 0) ? (ram_used / ram_total) : 0;
            nk_progress(nk_ctx, &ram_ratio, 100, NK_FIXED);

            nk_layout_row_dynamic(nk_ctx, 20, 2);
            nk_labelf(nk_ctx, NK_TEXT_LEFT, "Usage: %.1f%%", (ram_total > 0.0f) ? (ram_used / ram_total * 100.0f) : 0.0f);
            nk_labelf(nk_ctx, NK_TEXT_LEFT, "Total RAM: %.2f MB", ram_total);

            nk_tree_pop(nk_ctx);
        }

        // Render Stats
        if (nk_tree_push(nk_ctx, NK_TREE_NODE, "Render Stats", NK_MINIMIZED)) {
            nk_layout_row_dynamic(nk_ctx, 20, 2);

            static int draw_calls = 0;
            static int indices    = 0;
            static int vertices   = 0;
            nk_labelf(nk_ctx, NK_TEXT_LEFT, "Draw Calls %d", draw_calls);
            nk_labelf(nk_ctx, NK_TEXT_LEFT, "Vertices: %d", vertices);
            nk_labelf(nk_ctx, NK_TEXT_LEFT, "Indices: %d", indices);
            nk_checkbox_label(nk_ctx, "Show Render Targets", &show_render_targets);

            nk_tree_pop(nk_ctx);
        }


        // Profiler Data
        if (nk_tree_push(nk_ctx, NK_TREE_NODE, "Profiler", NK_MINIMIZED)) {
            nk_layout_row_dynamic(nk_ctx, 18, 1);
            nk_label(nk_ctx, "Frame Breakdown:", NK_TEXT_LEFT);

            // TODO: Add your profiler data here
            struct ProfilingData {
                const char* name;
                float time_ms;
                struct nk_color color;
            };

            static ProfilingData profile_data[] = {{"Update", 0.0f, nk_rgb(75, 150, 255)},
                                                   {"Render", 0.0f, nk_rgb(255, 150, 75)},
                                                   {"Physics", 0.0f, nk_rgb(150, 255, 75)},
                                                   {"Animations", 0.0f, nk_rgb(255, 75, 150)}};

            nk_layout_row_dynamic(nk_ctx, 2, 1);
            nk_spacing(nk_ctx, 1);

            for (const auto& data : profile_data) {
                nk_layout_row_dynamic(nk_ctx, 18, 2);

                nk_command_buffer* canvas = nk_window_get_canvas(nk_ctx);
                struct nk_rect bounds;
                if (nk_widget(&bounds, nk_ctx) != NK_WIDGET_INVALID) {
                    nk_fill_rect(canvas, nk_rect(bounds.x, bounds.y + 4, 10, 10), 2.0f, data.color);
                }

                nk_labelf(nk_ctx, NK_TEXT_RIGHT, "%.2f ms", data.time_ms);
            }

            nk_tree_pop(nk_ctx);
        }
    }
    nk_end(nk_ctx);


    if (show_render_targets) {
        if (nk_begin(nk_ctx, "Render Targets", nk_rect(830, 10, 400, 600),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            nk_label(nk_ctx, "Frame Buffer Outputs", NK_TEXT_CENTERED);

            const auto renderer = GEngine->get_renderer();

            auto depth_tex = renderer->get_shadow_map_fbo()->get_depth_attachment_id();
            if (depth_tex != 0) {
                nk_layout_row_dynamic(nk_ctx, 20, 1);
                nk_label(nk_ctx, "Depth Buffer:", NK_TEXT_LEFT);

                nk_layout_row_dynamic(nk_ctx, 256, 1);
                struct nk_image depth_img = nk_image_id((int) depth_tex);
                nk_image(nk_ctx, depth_img);
            }

            nk_end(nk_ctx);
        }
    }
}


void engine_core_loop() {
    static bool mouse_captured = false;
    static float mouse_dx      = 0.0f;
    static float mouse_dy      = 0.0f;

    GEngine->get_timer().tick();

    mouse_dx = 0.0f;
    mouse_dy = 0.0f;

    nk_input_begin(nk_ctx);
    while (SDL_PollEvent(&GEngine->event)) {
        auto& ev = GEngine->event;
        nk_sdl_handle_event(&ev);

        switch (ev.type) {
        case SDL_EVENT_QUIT:
            GEngine->is_running = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (ev.key.scancode == SDL_SCANCODE_ESCAPE) {
                mouse_captured = !mouse_captured;
                SDL_SetWindowRelativeMouseMode(GEngine->get_window(), mouse_captured);
                SDL_ShowCursor();
            }
            if (ev.key.scancode == SDL_SCANCODE_F1) {
                GEngine->get_config().is_debug = !GEngine->get_config().is_debug;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (ev.button.button == SDL_BUTTON_RIGHT && !mouse_captured) {
                mouse_captured = true;
                SDL_SetWindowRelativeMouseMode(GEngine->get_window(), true);
                SDL_HideCursor();
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            if (mouse_captured) {
                mouse_dx += static_cast<float>(ev.motion.xrel);
                mouse_dy += static_cast<float>(ev.motion.yrel);
            }
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            {

                spdlog::debug("Window resized to {}x{}", ev.window.data1, ev.window.data2);
                GEngine->get_config().get_window().resize(ev.window.data1, ev.window.data2);


                break;
            }

        default:
            break;
        }
    }
    nk_sdl_handle_grab();
    nk_input_end(nk_ctx);


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

        camera.speed = scancodes[SDL_SCANCODE_LSHIFT] ? 150.0f : 50.0f;

        if (mouse_captured && (mouse_dx != 0.0f || mouse_dy != 0.0f)) {
            constexpr float sensitivity = 0.1f;
            camera.look_at(mouse_dx, -mouse_dy, sensitivity);
        }
    });


    // draw_editor_ui();
    // draw_entity_hierarchy();
    draw_debug_ui();

    GEngine->get_world().progress(static_cast<float>(GEngine->get_timer().delta));
    engine_draw_loop();

    SDL_Delay(16); // Simple frame cap to ~60 FPS
}


void Engine::run() const {

#if defined(SDL_PLATFORM_EMSCRIPTEN)
    emscripten_set_main_loop(engine_core_loop, 60, 1);
#else
    while (is_running) {
        engine_core_loop();
    }

#endif
}


Engine::~Engine() {

    delete _renderer;

    SDL_DestroyWindow(_window);

    nk_sdl_shutdown();

    TTF_Quit();
    SDL_Quit();
}
