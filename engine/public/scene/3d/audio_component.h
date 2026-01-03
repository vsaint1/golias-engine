#pragma once
#include "scene/component.h"

namespace golias {
    class Audio;

    class AudioComponent : public Component {
        COMPONENT(AudioComponent)
    public:
        AudioComponent();
        ~AudioComponent() override;

        void RegisterAudio(const std::string_view pName, const std::shared_ptr<Audio>& pAudio);

        void Play(const std::string_view pName, bool loop = false);
        void Stop(const std::string_view pName);

        bool IsPlaying(const std::string_view pName) const;

        void Start() override;
        void Update(float deltaTime) override;

        void LoadProperties(const nlohmann::json& json) override;

    private:
        std::unordered_map<std::string, std::shared_ptr<Audio>> audios;
    };
} // namespace golias
