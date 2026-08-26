#include "core/audio/audio_manager.h"

#include <miniaudio.h>

namespace golias {

    AudioManager::AudioManager() {
    }

    AudioManager::~AudioManager() {
        Shutdown();
    }

    bool AudioManager::Initialize() {
        mEngine = std::make_unique<ma_engine>();


        if (ma_engine_init(NULL, mEngine.get()) != MA_SUCCESS) {
            GOLIAS_LOG_ERROR("Failed to initialize audio engine.");
            mEngine.reset();
            return false;
        }

        return true;
    }

    void AudioManager::Shutdown() {
        if (mEngine) {
            ma_engine_uninit(mEngine.get());
            mEngine.reset();
        }
    }

    ma_engine* AudioManager::GetEngine() const {
        return mEngine.get();
    }

    void AudioManager::SetListenerPosition(float x, float y, float z) {
        if (mEngine) {
            ma_engine_listener_set_position(mEngine.get(), 0, x, y, z);
        }
    }
} // namespace golias
