#include "core/ember_core.h"

ma_engine engine;


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
    if (flags & SDL_WINDOW_OPENGL || flags & SDL_WINDOW_METAL) {
        flags &= ~SDL_WINDOW_OPENGL;
        flags &= ~SDL_WINDOW_METAL;
    }

    flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

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

    int bbWidth, bbHeight;
    SDL_GetWindowSizeInPixels(_window, &bbWidth, &bbHeight);

    auto hdpi_screen = [display_mode, bbWidth, bbHeight]() {
        if (display_mode->w == bbWidth && display_mode->h == bbHeight) {
            return true;
        }
        return false;
    };

#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_ANDROID)

    SDL_SetWindowFullscreen(_window, true);

#endif

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


bool InitAudio() {
    SDL_AudioDeviceID devId;

    ma_engine_config config = ma_engine_config_init();
    config.channels         = 2;
    config.sampleRate       = 48000;
    ma_result res           = ma_engine_init(&config, &engine);

    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to initialize MA engine backend %d", res);
        return false;
    }

    SDL_memset(&core.Audio.spec, 0, sizeof(SDL_AudioSpec));

    core.Audio.spec.freq     = config.sampleRate;
    core.Audio.spec.format   = SDL_AUDIO_F32;
    core.Audio.spec.channels = config.channels;
    
    res = ma_engine_start(&engine);

    if(res != MA_SUCCESS) {
        LOG_ERROR("Failed to start MA engine backend %d", res);
        return false;
    }

    ma_engine_set_volume(&engine, core.Audio.global_volume);

    LOG_INFO("Successfully MA engine backend %d", res);
    return true;
}

Music* Mix_LoadMusic(const std::string& file_Path) {
    Music* music = (Music*) SDL_malloc(sizeof(Music));

    std::string path = ASSETS_PATH + file_Path;

    ma_result res = ma_sound_init_from_file(&engine, path.c_str(), 0, 0, 0, &music->sound);

    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to load sound file %s, %d", path.c_str(), res);
        return nullptr;
    }

    float len = 0.0f;
    res       = ma_sound_get_length_in_seconds(&music->sound, &len);


    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to get sound time %s, %d", path.c_str(), res);
        return nullptr;
    }

    music->duration = len;
    music->volume   = 1.f;

    LOG_INFO("Music loaded %s", path.c_str());
    LOG_INFO(" > Duration %.2f", music->duration / 60.f);
    LOG_INFO(" > Volume %.2f", music->volume);

    musics.emplace(path, music);

    return music;
}

void Mix_PlayMusic(Music* music, bool loop) {

    if (music == nullptr) {
        LOG_ERROR("The music wasn't loaded");
        return;
    }

    ma_sound_set_looping(&music->sound, loop);
    ma_sound_set_volume(&music->sound, music->volume);
    ma_sound_start(&music->sound);
}


void Mix_PauseMusic(Music* music) {

    if (music == nullptr) {
        LOG_ERROR("The music wasn't loaded");
        return;
    }

    ma_sound_stop(&music->sound);
}

void Mix_UnloadMusic(Music* music) {

    Mix_PauseMusic(music);

    ma_sound_uninit(&music->sound);

    SDL_free(music);
}

void CloseAudio() {

    LOG_INFO("Cleaning allocated Musics");

    for (auto& [_, music] : musics) {
        Mix_UnloadMusic(music);
    }

    LOG_INFO("Closing MA engine backend");
    ma_engine_uninit(&engine);

    // SDL_CloseAudioDevice(core.Audio.device_id);
}
