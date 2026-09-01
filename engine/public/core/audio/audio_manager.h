#pragma once

#include "stdafx.h"

struct ma_engine;

namespace golias {

    class AudioManager {
    public:
        AudioManager();
        ~AudioManager();

        bool Initialize();

        float GetMasterVolume() const;
        void SetMasterVolume(float volume);

        void Shutdown();

        ma_engine* GetEngine() const;

        void SetListenerPosition(float x, float y, float z);

    private:
        Scope<ma_engine> mEngine = nullptr;

        float mMasterVolume = 1.0f;
    };

} // namespace golias
