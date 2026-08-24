#pragma once

#include "component.h"
#include "stdafx.h"

namespace golias {

    struct KeyFrameVec3 {
        glm::vec3 Value = glm::vec3(0.0f);
        float Time      = 0.0f;
    };

    struct KeyFrameQuat {
        glm::quat Value = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        float Time      = 0.0f;
    };

    struct TransformTrack {
        String Name;
        std::vector<KeyFrameVec3> PositionKeyFrames;
        std::vector<KeyFrameQuat> RotationKeyFrames;
        std::vector<KeyFrameVec3> ScaleKeyFrames;
    };


    struct AnimationClip {
        String Name;
        float Duration = 0.0f;
        bool Looping   = false;
        std::vector<TransformTrack> Tracks;
    };

    struct ObjectBinding {
        GameObject* Object = nullptr;
        std::vector<size_t> TrackIndices; // Indices of the tracks in the AnimationClip that affect this object
    };

    class AnimationComponent : public Component {
        COMPONENT(AnimationComponent)
    public:
        AnimationComponent()          = default;
        virtual ~AnimationComponent() = default;

        void Update(float deltaTime) override;

        void SetClip(AnimationClip* clip);

        void RegisterClip(CString name, const Ref<AnimationClip>& clip);

        void Play(CString name, bool loop = true);

    private:
        void UpdateObjectBindings();

        glm::vec3 Interpolate(const std::vector<KeyFrameVec3>& keyFrames, float time);
        glm::quat Interpolate(const std::vector<KeyFrameQuat>& keyFrames, float time);

    private:
        AnimationClip* mCurrentClip = nullptr;
        float mCurrentTime          = 0.0f;
        bool mIsLooping             = true;
        bool mIsPlaying             = false;

        std::unordered_map<String, Ref<AnimationClip>> mAnimationClips;
        std::unordered_map<GameObject*, Scope<ObjectBinding>> mObjectBindings;
    };

} // namespace golias
