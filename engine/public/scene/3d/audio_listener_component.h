#pragma once
#include "scene/component.h"

namespace golias {
    class AudioListenerComponent : public Component {
        COMPONENT(AudioListenerComponent)
    public:
        AudioListenerComponent()  = default;
        ~AudioListenerComponent() = default;

        void Start() override;

        void Update(float deltaTime) override;

        void LoadProperties(const nlohmann::json& json) override {}
    };
} // namespace golias
