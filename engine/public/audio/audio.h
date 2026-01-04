#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


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
        std::vector<char> buffer;
        std::unique_ptr<ma_decoder> maDecoder;
        std::string path;

        bool spatializationEnabled = true;
        float volume = 0.5f;
        
        void PlayFireAndForget();
        
        static void OnSoundEnd(void* pUserData, ma_sound* pSound);
    };


}; // namespace golias
