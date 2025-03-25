#include "core/renderer/ember_core.h"


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

void Renderer::Destroy() {
    default_shader.Destroy();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    SDL_GL_DestroyContext(context);

    SDL_DestroyWindow(window);

    delete this;
}

bool InitWindow(const char* title, int width, int height, RendererType type, Uint64 flags) {

    /*!
        @brief Unset some SDL flags and set supported later.
    */
    if (flags & SDL_WINDOW_HIGH_PIXEL_DENSITY || flags & SDL_WINDOW_OPENGL || flags & SDL_WINDOW_METAL) {
        flags &= ~SDL_WINDOW_HIGH_PIXEL_DENSITY;
        flags &= ~SDL_WINDOW_OPENGL;
        flags &= ~SDL_WINDOW_METAL;
    }

    if (type == RendererType::METAL) {
        LOG_ERROR("Metal renderer is not supported yet");

        flags |= SDL_WINDOW_METAL;
        return false;
    }

    if (type == RendererType::OPENGL) {
        flags |= SDL_WINDOW_OPENGL;
    }

    // TODO: Get orientations
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        LOG_CRITICAL("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    /*!
        @brief Can have more than one display, but for now we will just use the first one
    */
    const SDL_DisplayMode* display_mode = SDL_GetDesktopDisplayMode(1);

    core.Window.data = display_mode;

    const char* _title = SDL_strcmp(title, "") == 0 ? title : core.Window.title;

    SDL_Window* _window = SDL_CreateWindow(_title, width, height, flags);

    if (!_window) {
        LOG_CRITICAL("Failed to create window: %s", SDL_GetError());
        return false;
    }

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID)

    SDL_SetWindowFullscreen(_window, true);

#endif

    renderer = CreateRenderer(_window, width, height, type);

    LOG_INFO("Successfully created window with title: %s", _title);
    LOG_INFO(" > Width %d, Height %d", width, height);
    LOG_INFO(" > Display ID %d", display_mode->displayID);
    LOG_INFO(" > Display Width %d, Display Height %d", display_mode->w, display_mode->h);
    LOG_INFO(" > Refresh Rate %.2f", display_mode->refresh_rate);
    LOG_INFO(" > Renderer %s", type == OPENGL ? "OpenGL" : "Metal");
    LOG_INFO(" > Viewport Width %d, Viewport Height %d", GetRenderer()->viewport[0],
             GetRenderer()->viewport[1]);

    core.Window.width  = width;
    core.Window.height = height;

    renderer->type = type;

    if (type == RendererType::OPENGL) {
        LOG_INFO(" > Version: %s", (const char*) glGetString(GL_VERSION));
        LOG_INFO(" > Vendor: %s", (const char*) glGetString(GL_VENDOR));
    }

    if (type == RendererType::METAL) {
        // TODO
    }


    return true;
}

void SetTargetFPS(int fps) {
    if (fps < 1) {
        core.Time.target = 0.0f;
    } else {
        core.Time.target = 1.0f / (float) fps;
    }

    core.Time.previous = SDL_GetTicks() / 1000.f;

    LOG_INFO("Target FPS (frames per second) to %02.03f ms", (float) core.Time.target * 1000.f);
}


void CloseWindow() {

    renderer->Destroy();


    SDL_Quit();
}
