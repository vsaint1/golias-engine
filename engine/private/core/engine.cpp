#include "core/engine.h"
#include "core/renderer/opengl/ogl_renderer.h"

std::unique_ptr<Engine> GEngine = std::make_unique<Engine>();

nk_context* nk_ctx = nullptr;

// TODO: remove all example/test functions (just hack here for testing purposes)

void example_2d_rendering() {
    auto* renderer = GEngine->get_renderer();

    static Camera2D camera;

    // Get virtual render resolution
    float vwidth  = static_cast<float>(renderer->get_virtual_width());
    float vheight = static_cast<float>(renderer->get_virtual_height());

    // Setup camera to match virtual resolution
    camera.viewport_size = glm::vec2(vwidth, vheight);
    camera.position      = glm::vec2(0, 0);
    camera.zoom          = 1.0f;

    static Transform2D camera_transform;

    renderer->begin_frame_2d();

    float margin_x = vwidth * 0.05f;
    float margin_y = vheight * 0.05f;

    renderer->draw_rect_2d(
        glm::vec2(margin_x, margin_y),
        glm::vec2(vwidth * 0.15f, vheight * 0.2f),
        glm::vec4(1, 0, 0, 1), // red color
        true
        );

    renderer->draw_text_2d(
        "Hello World!",
        glm::vec2(margin_x + vwidth * 0.2f, margin_y),
        glm::vec4(1, 0, 0, 1),
        1.0f
        );

    renderer->draw_text_2d(
        "Olá mundo",
        glm::vec2(margin_x, margin_y + vheight * 0.25f),
        glm::vec4(1, 1, 1, 1),
        1.0f
        );

    renderer->draw_text_2d(
        "Hello russian, мир! 🎮",
        glm::vec2(margin_x + vwidth * 0.08f, margin_y + vheight * 0.08f),
        glm::vec4(1, 1, 1, 1),
        1.0f
        );

    renderer->draw_text_2d(
        "😀🎮🚀💎✨",
        glm::vec2(margin_x, margin_y + vheight * 0.06f),
        glm::vec4(1, 1, 1, 1),
        1.0f
        );

    static int frame_count = 0;
    frame_count++;
    renderer->draw_text_2d_fmt(
        glm::vec2(vwidth * 0.5f, margin_y),
        glm::vec4(0, 0, 1, 1),
        1.0f,
        "Frame: {} | FPS: {:.1f}",
        frame_count,
        1.0 / GEngine->get_timer().delta
        );

    // Draw outlined rectangle (right-center area)
    renderer->draw_rect_2d(
        glm::vec2(vwidth * 0.65f, vheight * 0.3f),
        glm::vec2(vwidth * 0.25f, vheight * 0.2f),
        glm::vec4(0, 1, 0, 1), // green color
        false
        );

    // Draw a line (left side, diagonal)
    renderer->draw_line_2d(
        glm::vec2(margin_x, vheight * 0.5f),
        glm::vec2(vwidth * 0.35f, vheight * 0.65f),
        glm::vec4(0, 0, 1, 1), // blue color
        3.0f
        );

    // Draw a filled circle (center-left)
    renderer->draw_circle_2d(
        glm::vec2(vwidth * 0.25f, vheight * 0.5f),
        vheight * 0.1f,
        glm::vec4(1, 1, 0, 1), // yellow color
        true,
        32
        );

    // Draw a circle outline (center-right)
    renderer->draw_circle_outline_2d(
        glm::vec2(vwidth * 0.65f, vheight * 0.5f),
        vheight * 0.1f,
        glm::vec4(1, 0, 1, 1), // magenta color
        5.0f,
        32
        );

    // Draw a filled triangle (bottom-left)
    renderer->draw_triangle_2d(
        glm::vec2(margin_x, vheight * 0.85f),
        glm::vec2(margin_x + vwidth * 0.1f, vheight * 0.85f),
        glm::vec2(margin_x + vwidth * 0.05f, vheight * 0.7f),
        glm::vec4(0, 1, 1, 1), // cyan color
        true
        );

    // Draw an outlined triangle (bottom-center)
    renderer->draw_triangle_2d(
        glm::vec2(margin_x + vwidth * 0.15f, vheight * 0.85f),
        glm::vec2(margin_x + vwidth * 0.25f, vheight * 0.85f),
        glm::vec2(margin_x + vwidth * 0.2f, vheight * 0.7f),
        glm::vec4(1, 0.5, 0, 1), // orange color
        false
        );

    // Draw gradient triangles (bottom area)
    // int triangle_count = static_cast<int>(vwidth / 30.0f);
    // for (int i = 0; i < triangle_count && i < 100; ++i) {
    //     float x_pos = margin_x + vwidth * 0.3f + i * (vwidth * 0.3f) / triangle_count;
    //     float triangle_size = vwidth * 0.02f;
    //
    //     renderer->draw_triangle_2d(
    //         glm::vec2(x_pos, vheight * 0.85f),
    //         glm::vec2(x_pos + triangle_size, vheight * 0.85f),
    //         glm::vec2(x_pos + triangle_size * 0.5f, vheight * 0.75f),
    //         glm::vec4(0.1f * i / 10.0f, 0.2f * i / 10.0f, 0.3f * i / 10.0f, 1),
    //         true
    //     );
    // }

    // Draw texture (bottom-right)
    Uint32 gd_tex = renderer->load_texture_from_file("res/icon.png");
    renderer->draw_texture_2d(
        gd_tex,
        glm::vec2(vwidth * 0.75f, vheight * 0.7f),
        glm::vec2(vwidth * 0.1f, vheight * 0.15f)
        );

    renderer->end_frame_2d(camera, camera_transform);
}

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

    if (!renderer || !renderer->initialize(viewport.width, viewport.height, window)) {
        spdlog::error("Renderer initialization failed, shutting down");

        delete renderer;

        return nullptr;
    }

    renderer->load_font("res/fonts/Default.ttf", 24, "default");
    renderer->load_font("res/fonts/Twemoji.ttf", 24, "emoji");
    renderer->set_render_resolution(1280, 720);
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
    std::vector<DirectionalLight3D> directionalLights;
    glm::mat4 light_space_matrix(1.0f);

    Camera3D main_camera;
    Transform3D camera_transform;
    GEngine->get_world().each([&](const Camera3D& cam, const Transform3D& transform) {
        main_camera      = cam;
        camera_transform = transform;
    });

    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, DirectionalLight3D& light) {
        directionalLights.push_back(light);

        if (light.castShadows && light_space_matrix == glm::mat4(1.0f)) {
            const auto width   = GEngine->get_config().get_window().width;
            const auto height  = GEngine->get_config().get_window().height;
            light_space_matrix = light.get_light_space_matrix();
            // light_space_matrix = light.get_light_space_matrix(
            //         main_camera.get_view(camera_transform),
            //         main_camera.get_projection(width, height));
        }
    });

    std::vector<std::pair<Transform3D, SpotLight3D>> spotLights;
    GEngine->get_world().each([&](flecs::entity e, Transform3D& t, SpotLight3D& light) {
        spotLights.emplace_back(t, light);
    });


    const auto renderer = GEngine->get_renderer();

    renderer->begin_frame();
    auto query = GEngine->get_world().query<const Transform3D, const MeshRef, const MaterialRef>();

    query.each([&](const Transform3D& transform,
                   const MeshRef& mesh,
                   const MaterialRef& material) {
        renderer->add_to_render_batch(transform, mesh, material);
        renderer->add_to_shadow_batch(transform, mesh);
    });

    renderer->begin_shadow_pass();
    renderer->render_shadow_pass(light_space_matrix);
    renderer->end_shadow_pass();

    renderer->begin_render_target();
    renderer->render_main_target(main_camera, camera_transform, light_space_matrix, directionalLights, spotLights);
    renderer->end_render_target();

    example_2d_rendering();

    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

    GEngine->get_renderer()->swap_chain();

}


nk_bool show_render_targets = false;


void draw_editor_ui() {
    const float menu_height              = 25.0f;
    const float viewport_controls_height = 40.0f;
    const float scene_width              = 250.0f;
    const float properties_width         = 300.0f;
    const float content_height           = 200.0f;

    struct nk_rect bounds = {0, 0, 1280, 720};
    float window_width    = bounds.w;
    float window_height   = bounds.h;

    // Top Menu Bar
    if (nk_begin(nk_ctx, "MenuBar",
                 nk_rect(0, 0, window_width, menu_height),
                 NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {

        nk_menubar_begin(nk_ctx);
        nk_layout_row_begin(nk_ctx, NK_STATIC, 25, 8);

        // File Menu
        nk_layout_row_push(nk_ctx, 45);
        if (nk_menu_begin_label(nk_ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 200))) {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_menu_item_label(nk_ctx, "New Scene", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Open Scene", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Save Scene", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Save As...", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_item_label(nk_ctx, "----------", NK_TEXT_LEFT);
            if (nk_menu_item_label(nk_ctx, "Exit", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_end(nk_ctx);
        }

        // Edit Menu
        nk_layout_row_push(nk_ctx, 45);
        if (nk_menu_begin_label(nk_ctx, "Edit", NK_TEXT_LEFT, nk_vec2(120, 200))) {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_menu_item_label(nk_ctx, "Undo", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Redo", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_item_label(nk_ctx, "----------", NK_TEXT_LEFT);
            if (nk_menu_item_label(nk_ctx, "Cut", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Copy", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Paste", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Delete", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_end(nk_ctx);
        }

        // GameObject Menu
        nk_layout_row_push(nk_ctx, 80);
        if (nk_menu_begin_label(nk_ctx, "GameObject", NK_TEXT_LEFT, nk_vec2(150, 250))) {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_menu_item_label(nk_ctx, "Create Empty", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_item_label(nk_ctx, "----------", NK_TEXT_LEFT);
            if (nk_menu_item_label(nk_ctx, "3D Object >", NK_TEXT_LEFT)) {
                /* Submenu TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Light >", NK_TEXT_LEFT)) {
                /* Submenu TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Camera", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Audio Source", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_end(nk_ctx);
        }

        // Component Menu
        nk_layout_row_push(nk_ctx, 80);
        if (nk_menu_begin_label(nk_ctx, "Component", NK_TEXT_LEFT, nk_vec2(150, 200))) {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_menu_item_label(nk_ctx, "Add Component", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_item_label(nk_ctx, "----------", NK_TEXT_LEFT);
            if (nk_menu_item_label(nk_ctx, "Physics", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Rendering", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Audio", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_end(nk_ctx);
        }

        // Window Menu
        nk_layout_row_push(nk_ctx, 60);
        if (nk_menu_begin_label(nk_ctx, "Window", NK_TEXT_LEFT, nk_vec2(150, 200))) {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_menu_item_label(nk_ctx, "Scene", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Game", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Inspector", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Hierarchy", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Project", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Console", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_end(nk_ctx);
        }

        // View Menu
        nk_layout_row_push(nk_ctx, 45);
        if (nk_menu_begin_label(nk_ctx, "View", NK_TEXT_LEFT, nk_vec2(150, 150))) {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_menu_item_label(nk_ctx, "Reset Layout", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Fullscreen", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_end(nk_ctx);
        }

        // Build Menu
        nk_layout_row_push(nk_ctx, 45);
        if (nk_menu_begin_label(nk_ctx, "Build", NK_TEXT_LEFT, nk_vec2(150, 150))) {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_menu_item_label(nk_ctx, "Build Settings", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "Build and Run", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_end(nk_ctx);
        }

        // Help Menu
        nk_layout_row_push(nk_ctx, 45);
        if (nk_menu_begin_label(nk_ctx, "Help", NK_TEXT_LEFT, nk_vec2(150, 150))) {
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_menu_item_label(nk_ctx, "Documentation", NK_TEXT_LEFT)) {
                /* TODO */
            }
            if (nk_menu_item_label(nk_ctx, "About", NK_TEXT_LEFT)) {
                /* TODO */
            }
            nk_menu_end(nk_ctx);
        }

        nk_layout_row_end(nk_ctx);
        nk_menubar_end(nk_ctx);
    }
    nk_end(nk_ctx);

    float content_y       = window_height - content_height;
    float viewport_height = content_y - menu_height - viewport_controls_height;

    // Scene Hierarchy (Left Panel)
    if (nk_begin(nk_ctx, "Scene",
                 nk_rect(0, menu_height, scene_width, viewport_height + viewport_controls_height),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {

        nk_layout_row_dynamic(nk_ctx, 25, 2);
        if (nk_button_label(nk_ctx, "Create")) {
            /* TODO */
        }
        if (nk_button_label(nk_ctx, "Delete")) {
            /* TODO */
        }

        nk_layout_row_dynamic(nk_ctx, 20, 1);

        // Scene tree structure
        if (nk_tree_push(nk_ctx, NK_TREE_NODE, "Main Scene", NK_MAXIMIZED)) {
            if (nk_tree_push(nk_ctx, NK_TREE_NODE, "Main Camera", NK_MINIMIZED)) {
                nk_label(nk_ctx, "  Camera Component", NK_TEXT_LEFT);
                nk_tree_pop(nk_ctx);
            }

            if (nk_tree_push(nk_ctx, NK_TREE_NODE, "Directional Light", NK_MINIMIZED)) {
                nk_label(nk_ctx, "  Light Component", NK_TEXT_LEFT);
                nk_tree_pop(nk_ctx);
            }

            if (nk_tree_push(nk_ctx, NK_TREE_NODE, "Player", NK_MINIMIZED)) {
                nk_label(nk_ctx, "  Transform", NK_TEXT_LEFT);
                nk_label(nk_ctx, "  Mesh Renderer", NK_TEXT_LEFT);
                nk_label(nk_ctx, "  Rigidbody", NK_TEXT_LEFT);
                nk_tree_pop(nk_ctx);
            }

            if (nk_tree_push(nk_ctx, NK_TREE_NODE, "Environment", NK_MINIMIZED)) {
                nk_label(nk_ctx, "  Ground", NK_TEXT_LEFT);
                nk_label(nk_ctx, "  Sky", NK_TEXT_LEFT);
                nk_tree_pop(nk_ctx);
            }

            nk_tree_pop(nk_ctx);
        }
    }
    nk_end(nk_ctx);

    // Viewport (Center)
    float viewport_x     = scene_width;
    float viewport_width = window_width - scene_width - properties_width;

    if (nk_begin(nk_ctx, "Viewport",
                 nk_rect(viewport_x, menu_height, viewport_width, viewport_controls_height),
                 NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {

        nk_layout_row_begin(nk_ctx, NK_STATIC, 30, 6);

        // Play/Pause/Stop controls
        nk_layout_row_push(nk_ctx, 50);
        if (nk_button_label(nk_ctx, "Play")) {
            /* TODO: Start game */
        }

        nk_layout_row_push(nk_ctx, 50);
        if (nk_button_label(nk_ctx, "Pause")) {
            /* TODO: Pause game */
        }

        nk_layout_row_push(nk_ctx, 50);
        if (nk_button_label(nk_ctx, "Stop")) {
            /* TODO: Stop game */
        }

        nk_layout_row_push(nk_ctx, 20);
        nk_spacing(nk_ctx, 1);

        // Transform tools
        nk_layout_row_push(nk_ctx, 50);
        if (nk_button_label(nk_ctx, "Move")) {
            /* TODO: Move tool */
        }

        nk_layout_row_push(nk_ctx, 50);
        if (nk_button_label(nk_ctx, "Rotate")) {
            /* TODO: Rotate tool */
        }

        nk_layout_row_push(nk_ctx, 50);
        if (nk_button_label(nk_ctx, "Scale")) {
            /* TODO: Scale tool */
        }

        nk_layout_row_end(nk_ctx);
    }
    nk_end(nk_ctx);

    // Viewport Render Area
    if (nk_begin(nk_ctx, "ViewportRender",
                 nk_rect(viewport_x, menu_height + viewport_controls_height, viewport_width, viewport_height),
                 NK_WINDOW_BORDER)) {

        nk_layout_row_dynamic(nk_ctx, viewport_height - 40, 1);

        struct nk_rect bounds;
        if (nk_widget(&bounds, nk_ctx) != NK_WIDGET_INVALID) {
            struct nk_image viewport_image;
            viewport_image.w = (int) viewport_width - 20;
            viewport_image.h = (int) viewport_height - 20;

            // Get the framebuffer texture and display it (flipped vertically)
            struct nk_image depth_img = nk_image_id((int) GEngine->get_renderer()->get_main_fbo()->get_color_attachment_id());

            // Flip the image vertically by using a custom region
            struct nk_rect src_region = nk_rect(0, viewport_image.h, viewport_image.w, -viewport_image.h);
            nk_draw_image(nk_window_get_canvas(nk_ctx), bounds, &depth_img, {255, 255, 255, 255});
        }


    }
    nk_end(nk_ctx);

    // Properties/Inspector (Right Panel)
    if (nk_begin(nk_ctx, "Properties",
                 nk_rect(window_width - properties_width, menu_height, properties_width, viewport_height + viewport_controls_height),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {

        nk_layout_row_dynamic(nk_ctx, 25, 1);
        nk_label(nk_ctx, "Inspector", NK_TEXT_CENTERED);

        // Transform Component
        if (nk_tree_push(nk_ctx, NK_TREE_NODE, "Transform", NK_MAXIMIZED)) {
            nk_layout_row_dynamic(nk_ctx, 20, 2);

            static float pos[3]   = {0.0f, 0.0f, 0.0f};
            static float rot[3]   = {0.0f, 0.0f, 0.0f};
            static float scale[3] = {1.0f, 1.0f, 1.0f};

            nk_label(nk_ctx, "Position", NK_TEXT_LEFT);
            nk_spacing(nk_ctx, 1);

            nk_layout_row_dynamic(nk_ctx, 20, 3);
            nk_property_float(nk_ctx, "X:", -1000.0f, &pos[0], 1000.0f, 0.1f, 0.1f);
            nk_property_float(nk_ctx, "Y:", -1000.0f, &pos[1], 1000.0f, 0.1f, 0.1f);
            nk_property_float(nk_ctx, "Z:", -1000.0f, &pos[2], 1000.0f, 0.1f, 0.1f);

            nk_layout_row_dynamic(nk_ctx, 20, 2);
            nk_label(nk_ctx, "Rotation", NK_TEXT_LEFT);
            nk_spacing(nk_ctx, 1);

            nk_layout_row_dynamic(nk_ctx, 20, 3);
            nk_property_float(nk_ctx, "X:", -360.0f, &rot[0], 360.0f, 1.0f, 1.0f);
            nk_property_float(nk_ctx, "Y:", -360.0f, &rot[1], 360.0f, 1.0f, 1.0f);
            nk_property_float(nk_ctx, "Z:", -360.0f, &rot[2], 360.0f, 1.0f, 1.0f);

            nk_layout_row_dynamic(nk_ctx, 20, 2);
            nk_label(nk_ctx, "Scale", NK_TEXT_LEFT);
            nk_spacing(nk_ctx, 1);

            nk_layout_row_dynamic(nk_ctx, 20, 3);
            nk_property_float(nk_ctx, "X:", 0.01f, &scale[0], 100.0f, 0.1f, 0.1f);
            nk_property_float(nk_ctx, "Y:", 0.01f, &scale[1], 100.0f, 0.1f, 0.1f);
            nk_property_float(nk_ctx, "Z:", 0.01f, &scale[2], 100.0f, 0.1f, 0.1f);

            nk_tree_pop(nk_ctx);
        }

        // Mesh Renderer Component
        if (nk_tree_push(nk_ctx, NK_TREE_NODE, "Mesh Renderer", NK_MINIMIZED)) {
            nk_layout_row_dynamic(nk_ctx, 20, 2);

            static int cast_shadows    = 1;
            static int receive_shadows = 1;

            nk_checkbox_label(nk_ctx, "Cast Shadows", &cast_shadows);
            nk_checkbox_label(nk_ctx, "Receive Shadows", &receive_shadows);

            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_button_label(nk_ctx, "Select Material")) {
                /* TODO */
            }

            nk_tree_pop(nk_ctx);
        }

        // Add Component Button
        nk_layout_row_dynamic(nk_ctx, 30, 1);
        if (nk_button_label(nk_ctx, "+ Add Component")) {
            /* TODO */
        }
    }
    nk_end(nk_ctx);

    // Content Browser (Bottom Panel)
    if (nk_begin(nk_ctx, "Content",
                 nk_rect(0, content_y, window_width, content_height),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {

        nk_layout_row_begin(nk_ctx, NK_STATIC, 25, 4);
        nk_layout_row_push(nk_ctx, 80);
        if (nk_button_label(nk_ctx, "Assets")) {
            /* TODO */
        }
        nk_layout_row_push(nk_ctx, 80);
        if (nk_button_label(nk_ctx, "Textures")) {
            /* TODO */
        }
        nk_layout_row_push(nk_ctx, 80);
        if (nk_button_label(nk_ctx, "Models")) {
            /* TODO */
        }
        nk_layout_row_push(nk_ctx, 80);
        if (nk_button_label(nk_ctx, "Scripts")) {
            /* TODO */
        }
        nk_layout_row_end(nk_ctx);

        nk_layout_row_dynamic(nk_ctx, 2, 1);
        nk_spacing(nk_ctx, 1);

        // Asset grid view
        nk_layout_row_begin(nk_ctx, NK_STATIC, 80, 10);

        const char* assets[] = {
            "Cube.fbx", "Sphere.obj", "Material.mat", "Texture.png",
            "Player.lua", "Enemy.lua", "Scene.gscn", "Entity.gscn"
        };

        for (int i = 0; i < 8; i++) {
            nk_layout_row_push(nk_ctx, 80);

            struct nk_command_buffer* canvas = nk_window_get_canvas(nk_ctx);
            struct nk_rect bounds;
            if (nk_widget(&bounds, nk_ctx) != NK_WIDGET_INVALID) {
                // Asset thumbnail background
                nk_fill_rect(canvas, nk_rect(bounds.x, bounds.y, 70, 60), 2.0f, nk_rgb(60, 60, 65));

                // Asset icon placeholder
                nk_fill_rect(canvas, nk_rect(bounds.x + 10, bounds.y + 10, 50, 40), 1.0f, nk_rgb(80, 80, 85));

                // Asset name
                nk_draw_text(canvas, nk_rect(bounds.x, bounds.y + 62, 70, 15),
                             assets[i], strlen(assets[i]),
                             nk_ctx->style.font,
                             nk_rgb(60, 60, 65),
                             nk_rgb(200, 200, 200));
            }
        }

        nk_layout_row_end(nk_ctx);
    }
    nk_end(nk_ctx);
}

void draw_debug_ui() {

    if (nk_begin(nk_ctx, "Debug",
                 nk_rect(10, 10, 400, 600),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {
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

            nk_labelf(nk_ctx, NK_TEXT_LEFT, "%.1f%%",
                      (gpu_mem_total > 0.0f) ? (gpu_mem_used / gpu_mem_total * 100.0f) : 0.0f);

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

            static ProfilingData profile_data[] = {
                {"Update", 0.0f, nk_rgb(75, 150, 255)},
                {"Render", 0.0f, nk_rgb(255, 150, 75)},
                {"Physics", 0.0f, nk_rgb(150, 255, 75)},
                {"Animations", 0.0f, nk_rgb(255, 75, 150)}
            };

            nk_layout_row_dynamic(nk_ctx, 2, 1);
            nk_spacing(nk_ctx, 1);

            for (const auto& data : profile_data) {
                nk_layout_row_dynamic(nk_ctx, 18, 2);

                nk_command_buffer* canvas = nk_window_get_canvas(nk_ctx);
                struct nk_rect bounds;
                if (nk_widget(&bounds, nk_ctx) != NK_WIDGET_INVALID) {
                    nk_fill_rect(canvas,
                                 nk_rect(bounds.x, bounds.y + 4, 10, 10),
                                 2.0f, data.color);
                }

                nk_labelf(nk_ctx, NK_TEXT_RIGHT, "%.2f ms", data.time_ms);
            }

            nk_tree_pop(nk_ctx);
        }
    }
    nk_end(nk_ctx);


    if (show_render_targets) {
        if (nk_begin(nk_ctx, "Render Targets",
                     nk_rect(830, 10, 400, 600),
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

        case SDL_EVENT_WINDOW_RESIZED: {

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

        if (scancodes[SDL_SCANCODE_W])
            camera.move_forward(transform, dt);
        if (scancodes[SDL_SCANCODE_S])
            camera.move_backward(transform, dt);
        if (scancodes[SDL_SCANCODE_A])
            camera.move_left(transform, dt);
        if (scancodes[SDL_SCANCODE_D])
            camera.move_right(transform, dt);
        if (scancodes[SDL_SCANCODE_SPACE])
            transform.position.y += camera.speed * dt;
        if (scancodes[SDL_SCANCODE_LCTRL])
            transform.position.y -= camera.speed * dt;

        camera.speed = scancodes[SDL_SCANCODE_LSHIFT] ? 150.0f : 50.0f;

        if (mouse_captured && (mouse_dx != 0.0f || mouse_dy != 0.0f)) {
            constexpr float sensitivity = 0.1f;
            camera.look_at(mouse_dx, -mouse_dy, sensitivity);
        }

    });


    // draw_editor_ui();
    // draw_entity_hierarchy();
    // draw_debug_ui();

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
