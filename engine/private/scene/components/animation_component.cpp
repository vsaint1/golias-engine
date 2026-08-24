#include "scene/components/animation_component.h"

#include "scene/game_object.h"

namespace golias {

    void AnimationComponent::Update(float deltaTime) {

        if (!mCurrentClip) {
            return;
        }

        if (!mIsPlaying) {
            return;
        }

        mCurrentTime += deltaTime;

        if (mCurrentClip->Duration <= 0.0f) {
            mCurrentTime = 0.0f;
            mIsPlaying   = false;
        } else if (mCurrentTime > mCurrentClip->Duration) {
            if (mIsLooping) {
                mCurrentTime = fmod(mCurrentTime, mCurrentClip->Duration);
            } else {
                mCurrentTime = mCurrentClip->Duration;
                mIsPlaying   = false;
            }
        }

        for (auto& [object, binding] : mObjectBindings) {
            if (!object || !binding) {
                continue;
            }

            for (size_t i = 0; i < binding->TrackIndices.size(); ++i) {
                size_t trackIndex = binding->TrackIndices[i];
                if (trackIndex >= mCurrentClip->Tracks.size()) {
                    continue;
                }

                const TransformTrack& track = mCurrentClip->Tracks[trackIndex];

                if (!track.PositionKeyFrames.empty()) {
                    glm::vec3 position = Interpolate(track.PositionKeyFrames, mCurrentTime);
                    object->SetPosition(position);
                }

                if (!track.RotationKeyFrames.empty()) {
                    glm::quat rotation = Interpolate(track.RotationKeyFrames, mCurrentTime);
                    object->SetRotation(rotation);
                }

                if (!track.ScaleKeyFrames.empty()) {
                    glm::vec3 scale = Interpolate(track.ScaleKeyFrames, mCurrentTime);
                    object->SetScale(scale);
                }
            }
        }
    }

    glm::vec3 AnimationComponent::Interpolate(const std::vector<KeyFrameVec3>& keyFrames, float time) {

        if (keyFrames.empty()) {
            return glm::vec3(0.0f);
        }

        if (keyFrames.size() == 1) {
            return keyFrames[0].Value;
        }

        for (size_t i = 0; i < keyFrames.size() - 1; ++i) {
            const KeyFrameVec3& kf1 = keyFrames[i];
            const KeyFrameVec3& kf2 = keyFrames[i + 1];

            if (time >= kf1.Time && time <= kf2.Time) {
                float t = (time - kf1.Time) / (kf2.Time - kf1.Time);
                return glm::mix(kf1.Value, kf2.Value, t);
            }
        }

        return keyFrames.back().Value;
    }

    glm::quat AnimationComponent::Interpolate(const std::vector<KeyFrameQuat>& keyFrames, float time) {

        if (keyFrames.empty()) {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        if (keyFrames.size() == 1) {
            return keyFrames[0].Value;
        }

        for (size_t i = 0; i < keyFrames.size() - 1; ++i) {
            const KeyFrameQuat& kf1 = keyFrames[i];
            const KeyFrameQuat& kf2 = keyFrames[i + 1];

            if (time >= kf1.Time && time <= kf2.Time) {
                float t = (time - kf1.Time) / (kf2.Time - kf1.Time);
                return glm::slerp(kf1.Value, kf2.Value, t);
            }
        }

        return keyFrames.back().Value;
    }


    void AnimationComponent::SetClip(AnimationClip* clip) {
        mCurrentClip = clip;
        mCurrentTime = 0.0f;
        UpdateObjectBindings();
    }


    void AnimationComponent::RegisterClip(CString name, const Ref<AnimationClip>& clip) {
        mAnimationClips[name.data()] = clip;
    }

    void AnimationComponent::UpdateObjectBindings() {
        mObjectBindings.clear();

        if (!mCurrentClip) {
            return;
        }

        for (size_t i = 0; i < mCurrentClip->Tracks.size(); ++i) {
            const TransformTrack& track = mCurrentClip->Tracks[i];


            if (GameObject* object = GetOwner()->FindChildByName(track.Name)) {

                if (mObjectBindings.find(object) == mObjectBindings.end()) {
                    mObjectBindings[object]         = std::make_unique<ObjectBinding>();
                    mObjectBindings[object]->Object = object;
                }

                mObjectBindings[object]->TrackIndices.push_back(i);
            } else {
                GOLIAS_LOG_WARN("Could not find GameObject with name '{}' for track '{}'", track.Name, track.Name);
            }
        }
    }

    void AnimationComponent::Play(CString name, bool loop) {
        auto it = mAnimationClips.find(name.data());

        if (it != mAnimationClips.end()) {
            SetClip(it->second.get());
            mIsLooping = loop;
            mIsPlaying = true;
        }
    }

} // namespace golias
