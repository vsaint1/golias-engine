#include "core/ember_core.h"


Core core;
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
    SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);

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

    core.Time = new TimeManager();

    return true;
}


void CloseWindow() {

    renderer->Destroy();

    delete core.Time;

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

    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to start MA engine backend %d", res);
        return false;
    }

    ma_engine_set_volume(&engine, core.Audio.global_volume);

    LOG_INFO("Successfully MA engine backend");
    return true;
}

Audio* Mix_LoadAudio(const std::string& file_Path) {

    Audio* audio = (Audio*) SDL_malloc(sizeof(Audio));
    
    if(audio == nullptr){
        LOG_ERROR("Failed to allocate memory for sound");
        return nullptr;
    }

    std::string path = ASSETS_PATH + file_Path;

    ma_result res = ma_sound_init_from_file(&engine, path.c_str(), 0, 0, 0, &audio->sound);
    
    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to load sound file %s, %d", path.c_str(), res);
        return nullptr;
    }

    float len = 0.0f;
    res       = ma_sound_get_length_in_seconds(&audio->sound, &len);


    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to get sound time %s, %d", path.c_str(), res);
        return nullptr;
    }

    audio->duration = len;
    audio->volume   = 1.f;

    LOG_INFO("Audio loaded %s", file_Path.c_str());
    LOG_INFO(" > Duration %.2f seconds", audio->duration);
    LOG_INFO(" > Volume %.2f", audio->volume);

    audios.emplace(file_Path, audio);

    return audio;
}

void Mix_PlayAudio(Audio* audio, bool loop) {

    if (audio == nullptr) {
        LOG_ERROR("The Audio wasn't loaded");
        return;
    }

    ma_result res = ma_sound_start(&audio->sound);

    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to start Audio playback");
        return;
    }

    ma_sound_set_looping(&audio->sound, loop);
    ma_sound_set_volume(&audio->sound, audio->volume);

    if (!ma_sound_is_playing(&audio->sound)) {
        LOG_ERROR("Audio failed to start playing");
    }
}


void Mix_PauseAudio(Audio* audio) {

    if (audio == nullptr) {
        LOG_ERROR("The Audio wasn't loaded");
        return;
    }

    ma_sound_stop(&audio->sound);
}

void Mix_UnloadAudio(Audio* audio) {

    Mix_PauseAudio(audio);
    
    ma_sound_uninit(&audio->sound);

    SDL_free(audio);
}

void CloseAudio() {

    LOG_INFO("Cleaning allocated Audios");

    for (auto& [_, audio] : audios) {
        Mix_UnloadAudio(audio);
    }

    LOG_INFO("Closing MA engine backend");
    ma_engine_uninit(&engine);

    // SDL_CloseAudioDevice(core.Audio.device_id);
}
