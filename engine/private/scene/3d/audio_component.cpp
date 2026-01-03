#include "scene/3d/audio_component.h"

#include "audio/audio.h"
#include "scene/game_object.h"

namespace golias {

    AudioComponent::AudioComponent() {
    }
    AudioComponent::~AudioComponent() {
    }

    void AudioComponent::Start() {
    }


    void AudioComponent::Update(float deltaTime) {
        const auto& pos = GetOwner()->GetWorldPosition();

        for (auto& [name, audio] : audios) {
            if (audio->IsPlaying()) {
                audio->SetPosition(pos.x, pos.y, pos.z);
            }
        }
    }

    void AudioComponent::LoadProperties(const nlohmann::json& json) {

        if (json.contains("audio")) {
            auto& audioObjects = json["audio"];
            for (auto& audio : audioObjects) {
                std::string name = audio.value("name", "none");
                std::string path = audio.value("path", "");

                auto pAudio = Audio::Load(path);

                if (pAudio) {
                    float volume = audio.value("volume", 1.0f);
                    pAudio->SetVolume(volume);
                    
                    // Check if spatialization should be disabled (default is true/enabled)
                    bool spatial = audio.value("spatial", true);
                    pAudio->SetSpatialization(spatial);
                    
                    RegisterAudio(name, pAudio);
                }
            }
        }
    }


    void AudioComponent::RegisterAudio(const std::string_view pName, const std::shared_ptr<Audio>& pAudio) {
        audios[pName.data()] = pAudio;
    }

    void AudioComponent::Play(const std::string_view pName, bool loop) {
        auto it = audios.find(pName.data());
        
        if (it != audios.end()) {
            it->second->Play(loop);
        }

        
    }


    void AudioComponent::Stop(const std::string_view pName) {
        auto it = audios.find(pName.data());
        if (it != audios.end()) {
            it->second->Stop();
        }
    
    }

    bool AudioComponent::IsPlaying(const std::string_view pName) const {
        auto it = audios.find(pName.data());
        if (it != audios.end()) {
            return it->second->IsPlaying();
        }

        return false;
    }
} // namespace golias
