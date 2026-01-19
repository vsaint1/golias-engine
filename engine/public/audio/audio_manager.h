#pragma once
#include <memory>

struct ma_device;
struct ma_context;
struct rAudioBuffer;

namespace golias {

    class AudioManager {
    public:
        AudioManager();
        ~AudioManager();

        bool Initialize();

        ma_device* GetDevice();
        ma_context* GetContext();
        void* GetLock();

        void SetMasterVolume(float volume);
        float GetMasterVolume() const;

        // STUB for 3D audio listener position
        void SetListenerPosition(float x, float y, float z);

        void TrackAudioBuffer(rAudioBuffer* buffer);
        void UntrackAudioBuffer(rAudioBuffer* buffer);

        rAudioBuffer* GetFirstBuffer() {
            return firstBuffer;
        }

    private:
        ma_device* maDevice   = nullptr;
        ma_context* maContext = nullptr;

        void* maMutex = nullptr; // Opaque pointer (ma_mutex*)

        bool isReady       = false;
        float masterVolume = 1.0f;

        float listenerPosX = 0.0f;
        float listenerPosY = 0.0f;
        float listenerPosZ = 0.0f;

        rAudioBuffer* firstBuffer = nullptr;
        rAudioBuffer* lastBuffer  = nullptr;
    };
}; // namespace golias
