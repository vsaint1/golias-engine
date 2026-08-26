#pragma once

#include "scene/components/component.h"

namespace golias {

    class AudioListenerComponent : public Component {
        COMPONENT(AudioListenerComponent)
    public:
        AudioListenerComponent()           = default;
        ~AudioListenerComponent() override = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;
    };

} // namespace golias
