#pragma once

#include "scene/components/component.h"

namespace golias {

    class Audio;

    class AudioSourceComponent : public Component {
        COMPONENT(AudioSourceComponent)
    public:
        AudioSourceComponent()           = default;
        ~AudioSourceComponent() override = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        void OnEnable() override;

        void OnDisable() override;

        void RegisterAudio(CString name, const Ref<Audio>& audio);

        void Play(CString name, bool loop = false);
        void Pause(CString name);
        void Resume(CString name);
        void Stop(CString name);

        bool IsPlaying(CString name) const;

        float GetVolume(CString name) const;
        void SetVolume(CString name, float volume);

    private:
        std::unordered_map<String, Ref<Audio>> mAudios;

        std::vector<String> mPausedAudios;
    };

} // namespace golias
