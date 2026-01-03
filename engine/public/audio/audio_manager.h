#pragma once
#include <memory>

struct ma_engine;

namespace golias {

    class AudioManager {
    public:
        AudioManager();
        ~AudioManager();

        bool Initialize();

        ma_engine* GetNativeHandle();

        void SetListenerPosition(float x, float y, float z);

        void SetMasterVolume(float volume);

    private:
        std::unique_ptr<ma_engine> maEngine = nullptr;
        float masterVolume = 1.0f;
    };
}; // namespace golias
