#include "core/audio/ember_audio.h"


bool InitAudio() {
    SDL_AudioDeviceID devId;

    ma_engine_config config = ma_engine_config_init();
    config.channels         = 0;
    config.sampleRate       = 0;

    ma_result res = ma_engine_init(&config, &engine);

    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to initialize MA engine backend %d", res);
        return false;
    }

    SDL_memset(&core.Audio.spec, 0, sizeof(SDL_AudioSpec));

    core.Audio.spec.format   = SDL_AUDIO_F32;
    core.Audio.spec.freq     = SAMPLE_RATE;
    core.Audio.spec.channels = NUM_CHANNELS;

    res = ma_engine_start(&engine);

    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to start MA engine backend %d", res);
        return false;
    }

    ma_engine_set_volume(&engine, core.Audio.global_volume);

    Ember_VFS vfs;

    res = Ember_Init_VFS(&vfs);
    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to initialize MA engineVFS %d", res);
        return false;
    }

    core.ember_vfs = vfs;

    LOG_INFO("Successfully MA engine backend");
    return true;
}

Audio* Mix_LoadAudio(const std::string& file_Path) {

    Audio* audio = (Audio*) SDL_malloc(sizeof(Audio));

    SDL_assert(audio != nullptr);

    std::string path = ASSETS_PATH + file_Path;

    ma_decoder_config decoder_config = ma_decoder_config_init_default();

    ma_decoder* decoder = (ma_decoder*) SDL_malloc(sizeof(ma_decoder));

    if (!decoder) {
        LOG_ERROR("Failed to allocate memory for decoder");
        SDL_free(audio);
        return nullptr;
    }

    ma_result res = ma_decoder_init_vfs(&core.ember_vfs, path.c_str(), &decoder_config, decoder);
    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to decode sound file %s, error: %d", path.c_str(), res);
        SDL_free(decoder);
        SDL_free(audio);
        return nullptr;
    }

    res = ma_sound_init_from_data_source(&engine, decoder, 0, 0, &audio->sound);
    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to load sound file %s, error: %d", path.c_str(), res);
        ma_decoder_uninit(decoder);
        SDL_free(decoder);
        SDL_free(audio);
        return nullptr;
    }

    float len = 0.0f;
    res       = ma_sound_get_length_in_seconds(&audio->sound, &len);
    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to get sound duration for %s, error: %d", path.c_str(), res);
        ma_sound_uninit(&audio->sound);
        ma_decoder_uninit(decoder);
        SDL_free(decoder);
        SDL_free(audio);
        return nullptr;
    }

    audio->decoder  = decoder;
    audio->duration = len;
    audio->volume   = 0.5f;

    LOG_INFO("Audio loaded %s", file_Path.c_str());
    LOG_INFO(" > Duration %.2f seconds", audio->duration);
    LOG_INFO(" > Default Volume %.2f (0.0 - 1.0)", audio->volume);

    audios.emplace(file_Path, audio);

    return audio;
}

void Mix_PlayAudio(Audio* audio, bool loop) {

    SDL_assert(audio != nullptr);

    ma_uint64 time = ma_engine_get_time_in_milliseconds(&engine);

    ma_sound_set_fade_start_in_milliseconds(&audio->sound, 0.0f, audio->volume, 1000, time);

    ma_sound_set_looping(&audio->sound, loop);

    ma_sound_set_volume(&audio->sound, audio->volume);

    ma_result res = ma_sound_start(&audio->sound);

    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to start Audio playback");
        return;
    }

    if (!ma_sound_is_playing(&audio->sound)) {
        LOG_ERROR("Audio failed to start playing");
    }
}


void Mix_PauseAudio(Audio* audio) {

    SDL_assert(audio != nullptr);

    ma_sound_stop(&audio->sound);
}

void Mix_SetVolume(Audio* audio, float volume) {

    SDL_assert(audio != nullptr);

    volume = SDL_clamp(volume, 0.0f, 1.0f);

    ma_sound_set_volume(&audio->sound, volume);

    audio->volume = volume;
}

void Mix_SetGlobalVolume(float volume) {

    volume = SDL_clamp(volume, 0.0f, 100.f);

    core.Audio.global_volume = volume;
    ma_engine_set_volume(&engine, volume);
}

bool Mix_IsAudioPlaying(Audio* audio) {

    SDL_assert(audio != nullptr);

    return ma_sound_is_playing(&audio->sound);
}

void Mix_UnloadAudio(Audio* audio) {

    Mix_PauseAudio(audio);

    ma_sound_uninit(&audio->sound);

    SDL_free(audio->decoder);
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
