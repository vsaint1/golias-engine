#include "scene/components/audio_listener_component.h"
#include "core/engine.h"

namespace golias {

    bool AudioListenerComponent::LoadProperties(const Json& properties) {
      
        return true;
    }

    void AudioListenerComponent::Update(float deltaTime) {
        const glm::vec3 pos = GetOwner()->GetWorldPosition();
        Engine::GetInstance().GetAudioManager().SetListenerPosition(pos.x, pos.y, pos.z);
    }

} // namespace golias