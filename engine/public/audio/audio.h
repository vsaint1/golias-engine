#pragma once
#include <memory>
#include <string>
#include <unordered_map>


struct ma_sound;
struct ma_decoder;

namespace golias {


    class Audio {
    public:
        ~Audio();

        void SetPosition(float x, float y, float z);

        void Play(bool loop = false);

        void Stop();


        void SetVolume(float value);
        float GetVolume() const;

        bool IsPlaying() const;

        void SetSpatialization(bool enabled);


        static std::shared_ptr<Audio> Load(const std::string_view pPath);


    private:
        std::unique_ptr<ma_sound> maSound;
        std::unique_ptr<ma_decoder> maDecoder;
        std::vector<char> buffer;


        float volume = 1.0f;
    };


}; // namespace golias
