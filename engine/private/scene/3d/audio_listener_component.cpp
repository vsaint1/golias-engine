#include "scene/3d/audio_listener_component.h"

#include "core/engine.h"

namespace golias {
    void AudioListenerComponent::Start() {
    }

    void AudioListenerComponent::Update(float deltaTime) {
        const auto& pos = GetOwner()->GetWorldPosition();

        Engine::GetInstance().GetAudioManager().SetListenerPosition(pos.x, pos.y, pos.z);
    }
} // namespace golias
