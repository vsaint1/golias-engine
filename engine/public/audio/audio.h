#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct rAudioBuffer;

namespace golias {

    class Audio {
    public:
        ~Audio();

        void Play(bool loop = false);
        void Stop();
        void Pause();
        void Resume();

        void SetVolume(float value);
        float GetVolume() const;

        void SetPitch(float pitch);
        float GetPitch() const;

        void SetPan(float pan);
        float GetPan() const;

        bool IsPlaying() const;

        void SetPosition(float x, float y, float z);
        void SetSpatialization(bool enabled);

        static std::shared_ptr<Audio> Load(const std::string_view pPath);

    private:
        rAudioBuffer* buffer = nullptr;
        std::vector<char> data;
    };

}; // namespace golias
