#include "core/engine.h"

#include "core/audio/ember_audio.h"
#include "core/ember_core.h"
#include "core/renderer/opengl/ember_gl.h"

// 💀

std::unique_ptr<Engine> GEngine = std::make_unique<Engine>();

ma_engine audio_engine;



Renderer* Engine::CreateRenderer(SDL_Window* window, int view_width, int view_height, RendererType type) {

    if (type == RendererType::OPENGL) {
        return CreateRendererGL(window, view_width, view_height);
    }

    if (type == RendererType::METAL) {
        LOG_ERROR("Metal renderer is not supported yet");
        return nullptr;
    }


    LOG_CRITICAL("Unknown renderer type");

    return nullptr;
}


bool Engine::Initialize(const char* title, int width, int height, RendererType type, Uint64 flags) {

    LOG_INFO("Initializing %s, version %s", ENGINE_NAME, ENGINE_VERSION_STR);

    /*!
        @brief Unset some SDL flags and set supported later.
    */
    if (flags & SDL_WINDOW_OPENGL || flags & SDL_WINDOW_METAL) {
        flags &= ~SDL_WINDOW_OPENGL;
        flags &= ~SDL_WINDOW_METAL;
    }

    flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY; // (APPLE)
    flags |= SDL_WINDOW_HIDDEN;

    // TODO: check if metal is supported and create MTLDevice, if fail create OPENGL/ES
    if (type == RendererType::METAL) {
        LOG_ERROR("Metal renderer is not supported yet");

        flags |= SDL_WINDOW_METAL;
        return false;
    }

    if (type == RendererType::OPENGL) {
        flags |= SDL_WINDOW_OPENGL;
    }


#pragma region APP_METADATA
    // TODO: Get Metadata from config file
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft");
    SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);
    SDL_SetAppMetadata("Ember Engine", "1.0", "com.ember.engine");

#pragma endregion

    Debug::Start();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        LOG_CRITICAL("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }


    /*!
        @brief Can have more than one display, but for now we will just use the first one
    */
    const SDL_DisplayID displayID       = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* display_mode = SDL_GetDesktopDisplayMode(displayID);

    GEngine->Window.data = display_mode;

    const char* _title = SDL_strcmp(title, "") == 0 ? GEngine->Window.title : title;

    SDL_Window* _window = SDL_CreateWindow(_title, width, height, flags);

    if (!_window) {
        LOG_CRITICAL("Failed to create window: %s", SDL_GetError());
        return false;
    }

    if (!InitAudio()) {
        LOG_CRITICAL("Failed to initialize Audio Engine");
        return false;
    }

    LOG_INFO("Initialized Audio Engine");


    const std::string gamepad_mappings = LoadAssetsFile("controller_db");

    if (SDL_AddGamepadMapping(gamepad_mappings.c_str()) == -1) {
        LOG_CRITICAL("Failed to add gamepad mappings: %s", SDL_GetError());
        return false;
    }

    GEngine->Window.window = _window;

    int bbWidth, bbHeight;
    SDL_GetWindowSizeInPixels(_window, &bbWidth, &bbHeight);

    GEngine->Window.bbWidth  = bbWidth;
    GEngine->Window.bbHeight = bbHeight;

    auto hdpi_screen = [display_mode, bbWidth, bbHeight]() {
        if (display_mode->w == bbWidth && display_mode->h == bbHeight) {
            return true;
        }
        return false;
    };

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID)

    SDL_SetWindowFullscreen(_window, true);
    GEngine->Window.bFullscreen = true;

#endif

    this->renderer_ = CreateRenderer(_window, bbWidth, bbHeight, type);

    if (!this->renderer_) {
        LOG_CRITICAL("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    SDL_Rect view_bounds = {};
    SDL_GetDisplayUsableBounds(displayID, &view_bounds);

    LOG_INFO("Successfully created window with title: %s", _title);
    LOG_INFO(" > Width %d, Height %d", width, height);
    LOG_INFO(" > Display ID %d", display_mode->displayID);
    LOG_INFO(" > Display Width %d, Display Height %d", display_mode->w, display_mode->h);
    LOG_INFO(" > High DPI screen (%s), Backbuffer (%dx%d)", hdpi_screen() ? "YES" : "NO", bbWidth, bbHeight);
    LOG_INFO(" > Usable Bounds (%d, %d, %d, %d)", view_bounds.x, view_bounds.y, view_bounds.w, view_bounds.h);
    LOG_INFO(" > Refresh Rate %.2f", display_mode->refresh_rate);
    LOG_INFO(" > Renderer %s", type == OPENGL ? "OpenGL/ES" : "Metal");

    this->Window.width  = width;
    this->Window.height = height;
    this->Window.title  = _title;
    this->renderer_->type = type;

    if (type == RendererType::OPENGL) {
        LOG_INFO(" > Version: %s", (const char*) glGetString(GL_VERSION));
        LOG_INFO(" > Vendor: %s", (const char*) glGetString(GL_VENDOR));
    }

    if (type == RendererType::METAL) {
        // TODO
    }

    this->time_manager_  = new TimeManager();
    this->input_manager_ = new InputManager(_window);

    return true;
}


void Engine::Shutdown() {

    this->renderer_->Destroy();

    delete this->input_manager_;
    delete this->time_manager_;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    Debug::Destroy();

    CloseAudio();

    SDL_Quit();
}


void Engine::ResizeWindow(int w, int h) const {
    SDL_assert(w > 0 && h > 0);
    
    GEngine->Window.width= w;
    GEngine->Window.height = h;
}

Renderer* Engine::GetRenderer() const {
    return renderer_;
}
InputManager* Engine::GetInputManager() const {
    return input_manager_;
}

TimeManager* Engine::GetTimeManager() const {
    return time_manager_;
}
Renderer* Engine::CreateRendererGL(SDL_Window* window, int view_width, int view_height) {

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

    if (!SDL_GL_SetSwapInterval(0)) {
        LOG_CRITICAL("Failed to disable VSYNC, %s", SDL_GetError());
    }

    OpenglRenderer* glRenderer = new OpenglRenderer();
    glRenderer->viewport[0]    = view_width;
    glRenderer->viewport[1]    = view_height;
    glRenderer->window         = window;
    glRenderer->SetContext(glContext);

    glRenderer->default_shader = new OpenglShader("shaders/default.vert", "shaders/default.frag");
    glRenderer->text_shader    = new OpenglShader("shaders/sdf_text.vert", "shaders/sdf_text.frag");


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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, view_width, view_height);

    SDL_ShowWindow(window);

    this->renderer_ = glRenderer;

    return this->renderer_;
}
