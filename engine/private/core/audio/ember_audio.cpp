#include <utility>

#include "core/audio/ember_audio.h"

extern ma_engine audio_engine;

// ===================== AUDIO ===================== //

Audio* Audio::load(const std::string& file_Path, const std::string& busName) {
    const std::string path = ASSETS_PATH + file_Path;

    Audio* audio = (Audio*) SDL_malloc(sizeof(Audio));
    if (!audio) {
        LOG_ERROR("Failed to allocate audio %s", path.c_str());
        return nullptr;
    }

    ma_decoder_config decoder_config = ma_decoder_config_init_default();
    ma_decoder* decoder              = (ma_decoder*) SDL_malloc(sizeof(ma_decoder));

    if (!decoder) {
        LOG_ERROR("Failed to allocate decoder");
        SDL_free(audio);
        return nullptr;
    }

    ma_result res = ma_decoder_init_vfs(&GEngine->VirtualFileSystem, path.c_str(), &decoder_config, decoder);
    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to decode %s", path.c_str());
        SDL_free(decoder);
        SDL_free(audio);
        return nullptr;
    }

    res = ma_sound_init_from_data_source(&audio_engine, decoder, 0, nullptr, &audio->sound);
    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to init sound %s", path.c_str());
        ma_decoder_uninit(decoder);
        SDL_free(decoder);
        SDL_free(audio);
        return nullptr;
    }

    float len = 0.0f;
    ma_sound_get_length_in_seconds(&audio->sound, &len);
    audio->duration = len;
    audio->decoder  = decoder;
    audio->volume   = 1.0f;

    auto& bus = AudioBus::get_or_create(busName);
    bus.add_audio(audio);

    LOG_INFO("Loaded %s with Duration (%.2fs) into bus [%s]", file_Path.c_str(), audio->duration, busName.c_str());
    return audio;
}

void Audio::set_volume(float vol) {
    volume = SDL_clamp(vol, 0.0f, 1.0f);
    ma_sound_set_volume(&sound, volume);
}

void Audio::pause() {
    ma_sound_stop(&sound);
}

// TODO: if audiobus is muted, do not play
void Audio::play(bool loop) {

    ma_sound_set_looping(&sound, loop);

    if (ma_sound_start(&sound) != MA_SUCCESS) {
        LOG_ERROR("The Audio wasn't loaded properly, cannot play.");
    }

}

bool Audio::is_playing() {
    return ma_sound_is_playing(&sound);
}

void Audio::set_loop(bool loop) {
    ma_sound_set_looping(&sound, loop);
}

void Audio::destroy() {
    ma_sound_stop(&sound);
    ma_sound_uninit(&sound);
    ma_decoder_uninit(decoder);
    SDL_free(decoder);
    SDL_free(this);
}

// ===================== AUDIOBUS ===================== //

std::unordered_map<std::string, AudioBus> audioBuses;

AudioBus::AudioBus(std::string name) : _name(std::move(name)) {
}


void AudioBus::apply_volume(Audio* audio) {
    float effectiveVolume = is_muted ? 0.0f : audio->volume * volume;
    ma_sound_set_volume(&audio->sound, effectiveVolume);
}

AudioBus& AudioBus::get_or_create(const std::string& name) {
    const auto it = audioBuses.find(name);

    if (it == audioBuses.end()) {
        auto [newIt, _] = audioBuses.emplace(name, AudioBus(name));
        return newIt->second;
    }

    return it->second;
}

void AudioBus::add_audio(Audio* audio) {
    if (!audio) return;
    _sounds.push_back(audio);
    apply_volume(audio);
}

void AudioBus::remove_audio(Audio* audio) {
    std::erase(_sounds, audio);
}

void AudioBus::set_volume(float vol) {
    volume = SDL_clamp(vol, 0.0f, 1.0f);
    for (auto* s : _sounds) {
        apply_volume(s);
    }
}

void AudioBus::set_mute(bool mute) {
    is_muted = mute;
    for (auto* s : _sounds) {
        apply_volume(s);
    }
}

std::vector<Audio*> AudioBus::get_sounds() {
    return _sounds;
}

// ===================== ENGINE ===================== //

bool init_audio_engine() {
    LOG_INFO("Initializing audio engine");

    ma_device_config* device_config = (ma_device_config*) SDL_malloc(sizeof(ma_device_config));

    if (!device_config) {
        LOG_ERROR("Failed to allocate memory for device config %s", SDL_GetError());
        return false;
    }

    *device_config = ma_device_config_init(ma_device_type_playback);

    device_config->playback.format    = ma_format_f32;
    device_config->playback.channels  = 2;
    device_config->sampleRate         = 48000;
    device_config->dataCallback       = nullptr;
    device_config->periodSizeInFrames = 0;
    device_config->periods            = 0;
    device_config->performanceProfile = ma_performance_profile_low_latency;

    const ma_engine_config config = ma_engine_config_init();

    ma_result res = ma_engine_init(&config, &audio_engine);

    SDL_free(device_config);

    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to initialize MA engine backend %d", res);
        return false;
    }

    SDL_memset(&GEngine->Audio.spec, 0, sizeof(SDL_AudioSpec));
    GEngine->Audio.spec.format   = SDL_AUDIO_F32;
    GEngine->Audio.spec.freq     = 48000;
    GEngine->Audio.spec.channels = 2;


    res = ma_engine_start(&audio_engine);
    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to start MA engine backend %d", res);
        return false;
    }

    ma_engine_set_volume(&audio_engine, GEngine->Audio.global_volume);

    res = _ember_init_vfs(&GEngine->VirtualFileSystem);
    if (res != MA_SUCCESS) {
        LOG_ERROR("Failed to initialize MA engineVFS %d", res);
        return false;
    }

    // core.Audio.bInitialized = true;

    LOG_INFO("Successfully MA engine backend");
    return true;
}

void close_audio_engine() {

    for (auto [_, bus] : audioBuses) {
        LOG_INFO("Destroying audios from bus [%s]", bus._name.c_str());
        for (auto* audio : bus.get_sounds()) {
            audio->destroy();
        }
    }

    ma_engine_uninit(&audio_engine);
    LOG_INFO("Audio Engine Shutdown");
}

