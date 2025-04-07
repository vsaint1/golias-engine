#include "core/ember_core.h"


Renderer* CreateRenderer(SDL_Window* window, int view_width, int view_height, RendererType type) {

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

Renderer* GetRenderer() {
    return renderer;
}

void Renderer::SetContext(const SDL_GLContext& ctx) {
    context = ctx;
}

void Renderer::SetContext(/*MTL*/) {
    // TODO: metal
}

void Renderer::Resize(int w, int h) {
    viewport[0] = w;
    viewport[1] = h;
    glViewport(0, 0, w, h);
}


void Renderer::Destroy() {
    default_shader.Destroy();

    if (type == RendererType::OPENGL) {

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        SDL_GL_DestroyContext(context);
    }

    SDL_DestroyWindow(window);

    delete this;
}

bool InitWindow(const char* title, int width, int height, RendererType type, Uint64 flags) {

    /*!
        @brief Unset some SDL flags and set supported later.
    */
    if (flags & SDL_WINDOW_OPENGL || flags & SDL_WINDOW_METAL) {
        flags &= ~SDL_WINDOW_OPENGL;
        flags &= ~SDL_WINDOW_METAL;
    }

    flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY; // (APPLE)
    flags |= SDL_WINDOW_HIDDEN;

    if (type == RendererType::METAL) {
        LOG_ERROR("Metal renderer is not supported yet");

        flags |= SDL_WINDOW_METAL;
        return false;
    }

    if (type == RendererType::OPENGL) {
        flags |= SDL_WINDOW_OPENGL;
    }

    // TODO: Get orientations from config
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        LOG_CRITICAL("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }


    /*!
        @brief Can have more than one display, but for now we will just use the first one
    */
    const SDL_DisplayID displayID       = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* display_mode = SDL_GetDesktopDisplayMode(displayID);

    core.Window.data = display_mode;

    const char* _title = SDL_strcmp(title, "") == 0 ? core.Window.title : title;

    SDL_Window* _window = SDL_CreateWindow(_title, width, height, flags);

    if (!_window) {
        LOG_CRITICAL("Failed to create window: %s", SDL_GetError());
        return false;
    }
    
    const std::string gamepad_mappings = LoadAssetsFile("controller_db");

    if (SDL_AddGamepadMapping(gamepad_mappings.c_str()) == -1) {
        LOG_CRITICAL("Failed to add gamepad mappings: %s", SDL_GetError());
        return false;
    }

    core.Window.window = _window;

    int bbWidth, bbHeight;
    SDL_GetWindowSizeInPixels(_window, &bbWidth, &bbHeight);

    core.Window.bbWidth  = bbWidth;
    core.Window.bbHeight = bbHeight;

    auto hdpi_screen = [display_mode, bbWidth, bbHeight]() {
        if (display_mode->w == bbWidth && display_mode->h == bbHeight) {
            return true;
        }
        return false;
    };

    // #if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID)

    // //    SDL_SetWindowFullscreen(_window, true);
    //     core.Window.bFullscreen = true;

    // #endif

    renderer = CreateRenderer(_window, bbWidth, bbHeight, type);

    LOG_INFO("Successfully created window with title: %s", _title);
    LOG_INFO(" > Width %d, Height %d", width, height);
    LOG_INFO(" > Display ID %d", display_mode->displayID);
    LOG_INFO(" > Display Width %d, Display Height %d", display_mode->w, display_mode->h);
    LOG_INFO(" > High DPI screen (%s), Backbuffer (%dx%d)", hdpi_screen() ? "YES" : "NO", bbWidth, bbHeight);
    LOG_INFO(" > Refresh Rate %.2f", display_mode->refresh_rate);
    LOG_INFO(" > Renderer %s", type == OPENGL ? "OpenGL" : "Metal");
    // LOG_INFO(" > Viewport Width %d, Viewport Height %d", GetRenderer()->viewport[0], GetRenderer()->viewport[1]);

    core.Window.width  = width;
    core.Window.height = height;
    core.Window.title  = _title;
    renderer->type     = type;

    if (type == RendererType::OPENGL) {
        LOG_INFO(" > Version: %s", (const char*) glGetString(GL_VERSION));
        LOG_INFO(" > Vendor: %s", (const char*) glGetString(GL_VENDOR));
    }

    if (type == RendererType::METAL) {
        // TODO
    }

    core.Time  = new TimeManager();
    core.Input = new InputManager();

    core.Input->SetWindow(_window);

    return true;
}


void CloseWindow() {

    renderer->Destroy();

    delete core.Time;
    delete core.Input;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_Quit();
}
