#pragma once
#include "scene/component.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace golias {
    struct KeyFrameVec3 {
        float time      = 0.0f;
        glm::vec3 value = glm::vec3(0.0f);
    };

    struct KeyFrameQuat {
        float time      = 0.0f;
        glm::quat value = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    };

    struct TransformTrack {
        std::string targetName;
        std::vector<KeyFrameVec3> positions;
        std::vector<KeyFrameQuat> rotations;
        std::vector<KeyFrameVec3> scales;
    };

    struct AnimationClip {
        std::string name;
        float duration = 0.0f;
        bool looping   = true;
        std::vector<TransformTrack> tracks;
    };

    struct ObjectBinding {
        GameObject* object = nullptr;
        std::vector<size_t> trackIndices;
    };

    class AnimationComponent : public Component {
        COMPONENT(AnimationComponent)
    public:
        void Update(float deltaTime) override;
        void SetClip(AnimationClip* clip);
        void RegisterClip(const std::string& name, const std::shared_ptr<AnimationClip>& clip);
        void Play(const std::string& name, bool loop = true);

        bool IsPlaying() const;

        std::unordered_map<GameObject*, std::unique_ptr<ObjectBinding>>& GetBindings() {
            return bindings;
        }

        std::unordered_map<std::string, std::shared_ptr<AnimationClip>>& GetRegisteredClips() {
            return animation_clips;
        }

    private:
        void BuildBindings();
        glm::vec3 Interpolate(const std::vector<KeyFrameVec3>& keys, float time);
        glm::quat Interpolate(const std::vector<KeyFrameQuat>& keys, float time);

    private:
        AnimationClip* animation_clip = nullptr;
        float time                    = 0.0f;
        bool looping                  = true;
        bool playing                  = false;

        std::unordered_map<std::string, std::shared_ptr<AnimationClip>> animation_clips;
        std::unordered_map<GameObject*, std::unique_ptr<ObjectBinding>> bindings;
    };
} // namespace golias
