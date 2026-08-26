#pragma once

#include "stdafx.h"

struct ma_sound;
struct ma_decoder;

namespace golias {

    /// @brief  Represents an audio resource that can be loaded and played in the engine.
    class Audio {
    public:
        ~Audio();

        static Ref<Audio> Load(CString path);
        
        void Play(bool loop = false);
        void Stop();

        float GetVolume() const;
        void SetVolume(float volume);
 
        void SetPosition(float x, float y, float z);

        bool IsPlaying() const;

    private:
        Scope<ma_sound> mSound     = nullptr;
        Scope<ma_decoder> mDecoder = nullptr;

        std::vector<char> mAudioData;
    };
} // namespace golias
