#include "core/engine.h"

#include "core/audio/ember_audio.h"
#include "core/ember_core.h"
#include "core/renderer/metal/ember_mtl.h"
#include "core/renderer/opengl/ember_gl.h"
#include <stb_image.h>

// 💀

std::unique_ptr<Engine> GEngine = std::make_unique<Engine>();

ma_engine audio_engine;

void Engine::update(double delta_time) {

    this->_input_manager->update();
    this->_time_manager->update();

    b2World_Step(this->_world, SDL_min(delta_time, 1 / 60.f), 8);

    for (const auto&  system : _systems) {
        system->update(delta_time);
    }
}

ThreadPool& Engine::get_thread_pool() {
    return _thread_pool;
}

b2WorldId Engine::get_physics_world() const {
    return _world;
}


Renderer* Engine::_create_renderer_internal(SDL_Window* window, int view_width, int view_height, Backend type) {


    if (type == Backend::GL_COMPATIBILITY) {
        return _create_renderer_gl(window, view_width, view_height);
    }

    if (type == Backend::METAL) {
        return _create_renderer_metal(window, view_width, view_height);
    }


    LOG_CRITICAL("Unknown renderer type");

    return nullptr;
}

void Engine::set_vsync(const bool enabled) {

    if (_renderer->Type == Backend::GL_COMPATIBILITY) {
        SDL_GL_SetSwapInterval(enabled ? 1 : 0);
    } else if (_renderer->Type == Backend::METAL) {
        // TODO
    }

    Config.set_vsync(enabled);
}

bool Engine::initialize(int width, int height, Backend type, Uint64 flags) {

    // Note: curl is not supported on Emscripten/WebAssembly
#if !defined(SDL_PLATFORM_EMSCRIPTEN)
    curl_global_init(CURL_GLOBAL_DEFAULT);
#endif

    LOG_INFO("Initializing %s, version %s", ENGINE_NAME, ENGINE_VERSION_STR);

    if (!Config.load()) {
        LOG_CRITICAL("Failed to load config file (project.xml)");
    }

    const auto app_config = Config.get_application();

    if (app_config.is_fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    if (app_config.is_resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }


    /*!
        @brief Unset some SDL flags and set supported later.
    */
    if (flags & SDL_WINDOW_OPENGL || flags & SDL_WINDOW_METAL) {
        flags &= ~SDL_WINDOW_OPENGL;
        flags &= ~SDL_WINDOW_METAL;
    }

    // flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY; // MUST FIX HiDPI SUPPORT
    flags |= SDL_WINDOW_HIDDEN;

    // TODO: check if metal is supported and create MTLDevice, if fail create OPENGL/ES
    if (type == Backend::METAL) {
        LOG_INFO("Metal renderer is currently experimental, many features are not implemented yet");

        flags |= SDL_WINDOW_METAL;
    }

    if (type == Backend::GL_COMPATIBILITY) {
        flags |= SDL_WINDOW_OPENGL;
    }


#pragma region APP_METADATA
    SDL_SetHint(SDL_HINT_ORIENTATIONS, Config.get_orientation_str());
    SDL_SetAppMetadata(app_config.name, app_config.version, app_config.package_name);
#pragma endregion

    if (Config.get_threading().is_multithreaded) {
        Logger::initialize();
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        LOG_CRITICAL("Failed to initialize SDL: %s", SDL_GetError());

        if (Config.get_threading().is_multithreaded) {
            Logger::destroy();
        }

        return false;
    }


    /*!
        @brief Can have more than one display, but for now we will just use the first one
    */
    const SDL_DisplayID displayID       = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* display_mode = SDL_GetDesktopDisplayMode(displayID);

    this->Window.data = display_mode;

    SDL_Window* _window = SDL_CreateWindow(app_config.name, width, height, flags);

    if (!_window) {
        LOG_CRITICAL("Failed to create window: %s", SDL_GetError());

        if (Config.get_threading().is_multithreaded) {
            Logger::destroy();
        }

        return false;
    }

#pragma region APP_ICON


    int w, h, nr_channels;
    unsigned char* pixels = stbi_load(Config.get_application().icon_path, &w, &h, &nr_channels, 4);
    if (!pixels) {
        LOG_ERROR("Failed to load default icon");
    } else {

        SDL_Surface* icon = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, w * 4);

        if (icon) {
            SDL_SetWindowIcon(_window, icon);
            SDL_DestroySurface(icon);
        }

        stbi_image_free(pixels);
    }

#pragma endregion

#pragma region ENGINE_SYS
    _systems.emplace_back(std::make_unique<AudioManager>());
    _systems.emplace_back(std::make_unique<PhysicsManager>());

    for (const auto& system : _systems) {
        if (!system->initialize()) {
            LOG_CRITICAL("Failed to initialize system: %s", system->get_name());
            SDL_DestroyWindow(_window);

            if (Config.get_threading().is_multithreaded) {
                Logger::destroy();
            }

            return false;
        }
    }

#pragma endregion

    FileAccess file("controller_db", ModeFlags::READ);

    const std::string gamepad_mappings = file.get_file_as_str();

    if (SDL_AddGamepadMapping(gamepad_mappings.c_str()) == -1) {
        LOG_CRITICAL("Failed to add gamepad mappings: %s", SDL_GetError());
        SDL_DestroyWindow(_window);

        if (Config.get_threading().is_multithreaded) {
            Logger::destroy();
        }

        return false;
    }

    this->Window.handle = _window;

    int bbWidth, bbHeight;
    SDL_GetWindowSizeInPixels(_window, &bbWidth, &bbHeight);

    SDL_Rect view_bounds = {};
    SDL_GetDisplayUsableBounds(displayID, &view_bounds);

    this->Window.bbWidth  = bbWidth;
    this->Window.bbHeight = bbHeight;

    auto hdpi_screen = [display_mode, bbWidth, bbHeight]() {
        if (display_mode->w == bbWidth && display_mode->h == bbHeight) {
            return true;
        }
        return false;
    };

    const Viewport viewport = Config.get_viewport();

    this->_renderer = _create_renderer_internal(_window, viewport.width, viewport.height, type);

    if (!this->_renderer) {
        LOG_CRITICAL("Failed to create renderer: (unknown type)");
        SDL_DestroyWindow(_window);

        if (Config.get_threading().is_multithreaded) {
            Logger::destroy();
        }

        return false;
    }

    LOG_INFO(R"(Successfully created window with title: %s
     > Width %d, Height %d
     > Display ID %d
     > Display Width %d, Display Height %d
     > High DPI screen (%s), Backbuffer (%dx%d)
     > Usable Bounds (%d, %d, %d, %d)
     > Viewport (%d, %d)
     > Refresh Rate %.2f
     > Renderer %s)",
             app_config.name, width, height, display_mode->displayID, display_mode->w, display_mode->h, hdpi_screen() ? "YES" : "NO",
             bbWidth, bbHeight, view_bounds.x, view_bounds.y, view_bounds.w, view_bounds.h, viewport.width, viewport.height,
             display_mode->refresh_rate, Config.get_renderer_device().get_backend_str());

    this->Window.width    = width;
    this->Window.height   = height;
    this->_renderer->Type = type;

    if (type == Backend::GL_COMPATIBILITY) {
        LOG_INFO("Version: %s", (const char*) glGetString(GL_VERSION));
        LOG_INFO("Vendor: %s", (const char*) glGetString(GL_VENDOR));
    }

    //TODO: get this dyn.
    if (type == Backend::METAL) {
        LOG_INFO("Version: %s ", "Metal 2.0+");
        LOG_INFO("Vendor: %s", "Apple Inc.");
    }


    this->set_vsync(Config.is_vsync());

    this->_time_manager  = new TimeManager();
    this->_input_manager = new InputManager(_window);
    this->is_running     = true;

    this->_time_manager->set_target_fps(Config.get_application().max_fps);


    return true;
}


void Engine::shutdown() {

#if !defined(SDL_PLATFORM_EMSCRIPTEN)
    curl_global_cleanup();
#endif

    this->_renderer->destroy();

    delete this->_renderer;
    this->_renderer = nullptr;

    delete this->_input_manager;
    this->_input_manager = nullptr;

    delete this->_time_manager;
    this->_time_manager = nullptr;

    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (Config.get_threading().is_multithreaded) {
        Logger::destroy();
    }

    b2DestroyWorld(this->_world);

    SDL_DestroyWindow(Window.handle);

    _thread_pool.shutdown();

    for (const auto&  system : _systems) {
        system->shutdown();
    }

    _systems.clear();

    SDL_Quit();
}

Renderer* Engine::_create_renderer_metal(SDL_Window* window, int view_width, int view_height) {

    MetalRenderer* mtlRenderer = new MetalRenderer();
    mtlRenderer->Viewport[0]   = view_width;
    mtlRenderer->Viewport[1]   = view_height;
    mtlRenderer->Window        = window;
    mtlRenderer->Type          = Backend::METAL;

    mtlRenderer->initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    // TODO: initialize impl Metal

    SDL_ShowWindow(window);

    this->_renderer = mtlRenderer;

    return mtlRenderer;
}


Engine::Engine() : _thread_pool(2) {
    const b2WorldDef world_def = b2DefaultWorldDef();
    _world                     = b2CreateWorld(&world_def);
}


void Engine::resize_window(int w, int h) {
    SDL_assert(w > 0 && h > 0);

    this->Window.width  = w;
    this->Window.height = h;
    SDL_SetWindowSize(Window.handle, w, h);
}

Renderer* Engine::get_renderer() const {
    return _renderer;
}

InputManager* Engine::input_manager() const {
    return _input_manager;
}

TimeManager* Engine::time_manager() const {
    return _time_manager;
}

Renderer* Engine::_create_renderer_gl(SDL_Window* window, int view_width, int view_height) {

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    /* GLES 3.0 -> GLSL: 300 */
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

#elif defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)

    /* OPENGL 3.3 -> GLSL: 330*/
    const char* glsl_version = "#version 330";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if (!glContext) {
        LOG_CRITICAL("Failed to create GL context, %s", SDL_GetError());
        return nullptr;
    }


#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)

    if (!gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to initialize GLAD (GLES_FUNCTIONS)");
        return nullptr;
    }

#else

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to initialize GLAD (GL_FUNCTIONS)");
        return nullptr;
    }

#endif


#if ENGINE_DEBUG
    int numExtensions = 0;

    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    LOG_INFO("Number of available OpenGL/ES extensions: %d", numExtensions);
    for (int i = 0; i < numExtensions; i++) {
        const char* extension = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        LOG_INFO("GL_EXTENSION_NAME: %s", extension);
    }
#endif


    OpenglRenderer* glRenderer = new OpenglRenderer();

    glRenderer->Viewport[0] = view_width;
    glRenderer->Viewport[1] = view_height;
    glRenderer->Window      = window;
    glRenderer->Type        = Backend::GL_COMPATIBILITY;

    glRenderer->set_context(glContext);

    OpenglShader* defaultShader    = new OpenglShader("shaders/opengl/default.vert", "shaders/opengl/default.frag");
    OpenglShader* defaultFboShader = new OpenglShader("shaders/opengl/default_fbo.vert", "shaders/opengl/default_fbo.frag");

    glRenderer->setup_shaders(defaultShader, defaultFboShader);

    glRenderer->initialize();


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, view_width, view_height);

    SDL_ShowWindow(window);

    this->_renderer = glRenderer;


    return _renderer;
}


b2Vec2 pixels_to_world(const glm::vec2& pixelPos) {
    const  float viewportHeight = GEngine->Config.get_viewport().height;
    return b2Vec2(pixelPos.x * METERS_PER_PIXEL, (viewportHeight - pixelPos.y) * METERS_PER_PIXEL);
}

glm::vec2 world_to_pixels(const b2Vec2& worldPos) {
    const float viewportHeight = GEngine->Config.get_viewport().height;
    return glm::vec2(worldPos.x * PIXELS_PER_METER, viewportHeight - worldPos.y * PIXELS_PER_METER);
}
