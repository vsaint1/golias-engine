#include "audio/audio_manager.h"

#include <miniaudio.h>

namespace golias {


    AudioManager::AudioManager() {
        maEngine = std::make_unique<ma_engine>();
    }

    AudioManager::~AudioManager() {
        if (maEngine) {

            ma_engine_uninit(maEngine.get());
        }
    }

    bool AudioManager::Initialize() {

        auto res = ma_engine_init(NULL, maEngine.get());

        return res == MA_SUCCESS;
    }

    ma_engine* AudioManager::GetNativeHandle() {
        return maEngine.get();
    }

    void AudioManager::SetListenerPosition(float x, float y, float z) {
        if (maEngine) {
            ma_engine_listener_set_position(maEngine.get(), 0, x, y, z);
        }
    }


    void AudioManager::SetMasterVolume(float volume) {
        masterVolume = volume;
        if (maEngine) {
            ma_engine_set_volume(maEngine.get(), masterVolume);
        }
    }
} // namespace golias
