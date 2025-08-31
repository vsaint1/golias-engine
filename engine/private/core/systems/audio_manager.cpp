#include <utility>

#include "core/systems/audio_manager.h"

extern ma_engine audio_engine;

// ===================== ENGINE ===================== //

bool init_audio_engine() {
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
    AudioBus::get_or_create("Master");

    LOG_INFO("Successfully initialized Audio Engine, Default bus [Master] Volume %.2f", GEngine->Audio.global_volume);
    return true;
}

void close_audio_engine() {

    for (auto& [_, bus] : audioBuses) {
        LOG_INFO("Destroying audios from bus [%s]", bus._name.c_str());
        for (auto* audio : bus.get_sounds()) {
            audio->destroy();
        }
    }

    ma_engine_uninit(&audio_engine);
}

bool AudioManager::initialize() {
    LOG_INFO("AudioManager::initialize()");
    return init_audio_engine();
}

// PASS
void AudioManager::update(double delta_time) {
}

void AudioManager::shutdown() {
    LOG_INFO("AudioManager::shutdown()");

    close_audio_engine();
}

AudioManager::~AudioManager() = default;

