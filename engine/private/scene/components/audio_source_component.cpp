#include "scene/components/audio_source_component.h"

#include "core/audio/audio.h"
#include "scene/game_object.h"

namespace golias {

    bool AudioSourceComponent::LoadProperties(const Json& properties) {
        if (properties.contains("audios") && properties["audios"].is_array()) {
            const Json& audioProperties = properties["audios"];

            for (const auto& audioProp : audioProperties) {

                String name      = audioProp.value("name", "UnamedAudio");
                String path      = audioProp.value("path", "");
                bool playOnAwake = audioProp.value("play_on_start", false);

                Ref<Audio> audio = Audio::Load(path);

                if (audio) {
                    float volume = audioProp.value("volume", 1.0f);
                    audio->SetVolume(volume);
                    RegisterAudio(name, audio);

                    if (playOnAwake) {
                        audio->Play();
                    }

                } else {
                    GOLIAS_LOG_ERROR("Failed to load audio: %s", path.c_str());
                }
            }
        }

        return true;
    }

    void AudioSourceComponent::Update(float deltaTime) {
        const glm::vec3 pos = GetOwner()->GetWorldPosition();
        for (const auto& [name, audio] : mAudios) {
            audio->SetPosition(pos.x, pos.y, pos.z);
        }
    }

    void AudioSourceComponent::OnEnable() {
        for (const auto& name : mPausedAudios) {
            auto it = mAudios.find(name);
            if (it != mAudios.end()) {
                it->second->Resume();
            }
        }

        mPausedAudios.clear();
    }

    void AudioSourceComponent::OnDisable() {
        mPausedAudios.clear();

        for (const auto& [name, audio] : mAudios) {
            if (audio->IsPlaying()) {
                audio->Pause();
                mPausedAudios.push_back(name);
            }
        }
    }

    void AudioSourceComponent::RegisterAudio(CString name, const Ref<Audio>& audio) {
        mAudios[name.data()] = audio;
    }

    void AudioSourceComponent::Play(CString name, bool loop) {
        for (const auto& [audioName, audio] : mAudios) {
            if (audioName == name.data()) {
                audio->Play(loop);
                return;
            }
        }

        GOLIAS_LOG_ERROR("Audio with name '%s' not found.", name.data());
    }

    void AudioSourceComponent::Pause(CString name) {
        auto it = mAudios.find(name.data());
        if (it != mAudios.end()) {
            it->second->Pause();
            return;
        }

        GOLIAS_LOG_ERROR("Audio with name '%s' not found.", name.data());
    }

    void AudioSourceComponent::Resume(CString name) {
        auto it = mAudios.find(name.data());
        if (it != mAudios.end()) {
            it->second->Resume();
            return;
        }

        GOLIAS_LOG_ERROR("Audio with name '%s' not found.", name.data());
    }

    void AudioSourceComponent::Stop(CString name) {
        for (const auto& [audioName, audio] : mAudios) {
            if (audioName == name.data()) {
                audio->Stop();
                return;
            }
        }

        GOLIAS_LOG_ERROR("Audio with name '%s' not found.", name.data());
    }

    bool AudioSourceComponent::IsPlaying(CString name) const {

        auto it = mAudios.find(name.data());
        if (it != mAudios.end()) {
            const Ref<Audio>& audio = it->second;
            return audio->IsPlaying();
        }

        return false;
    }

    float AudioSourceComponent::GetVolume(CString name) const {
        auto it = mAudios.find(name.data());
        if (it != mAudios.end()) {
            const Ref<Audio>& audio = it->second;
            return audio->GetVolume();
        }

        GOLIAS_LOG_ERROR("Audio with name '%s' not found.", name.data());
        return 0.0f;
    }

    void AudioSourceComponent::SetVolume(CString name, float volume) {
        auto it = mAudios.find(name.data());
        if (it != mAudios.end()) {
            const Ref<Audio>& audio = it->second;
            audio->SetVolume(volume);
        } else {
            GOLIAS_LOG_ERROR("Audio with name '%s' not found.", name.data());
        }
    }

} // namespace golias
