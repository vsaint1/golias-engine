#include "scene/3d/skeleton_animation_component.h"

#include "scene/3d/animation_component.h"
#include "scene/game_object.h"
#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace golias {

    void SkeletonAnimationComponent::Start() {
        BuildTrackIndices();
        if (skeleton) {
            skeleton->jointMatrices.resize(skeleton->joints.size(), glm::mat4(1.0f));
        }
    }

    void SkeletonAnimationComponent::Update(float deltaTime) {
        if (!animationClip || !skeleton || !playing) {
            return;
        }

        time += deltaTime;

        if (time > animationClip->duration) {
            if (looping) {
                time = std::fmod(time, animationClip->duration);
            } else {
                time    = animationClip->duration;
                playing = false;
            }
        }

        for (auto& track : animationClip->tracks) {
            if (track.targetJointIndex < 0 || track.targetJointIndex >= (int) skeleton->joints.size()) {
                continue;
            }

            auto& joint = skeleton->joints[track.targetJointIndex];

            if (!track.positions.empty()) {
                joint.position = Interpolate(track.positions, time);
            }

            if (!track.rotations.empty()) {
                joint.rotation = Interpolate(track.rotations, time);
            }

            if (!track.scales.empty()) {
                joint.scale = Interpolate(track.scales, time);
            }
        }

        UpdateJointMatrices();
    }

    const std::vector<glm::mat4>& SkeletonAnimationComponent::GetJointMatrices() const {
        return skeleton->jointMatrices;
    }

    std::unordered_map<std::string, std::shared_ptr<SkeletonAnimationClip>>& SkeletonAnimationComponent::GetRegisteredClips() {
        return animationClips;
    }

    bool SkeletonAnimationComponent::IsPlaying() const {
        return playing;
    }

    void SkeletonAnimationComponent::UpdateJointMatrices() {
        if (!skeleton || skeleton->joints.empty()) {
            return;
        }

        std::vector<glm::mat4> globalTransforms(skeleton->joints.size(), glm::mat4(0.0f));
        std::vector<bool> computed(skeleton->joints.size(), false);

        for (size_t i = 0; i < skeleton->joints.size(); ++i) {
            ComputeGlobalTransformRecursive(static_cast<int>(i), globalTransforms, computed);
        }

        for (size_t i = 0; i < skeleton->joints.size(); ++i) {
            skeleton->jointMatrices[i] = globalTransforms[i] * skeleton->joints[i].inverseBindMatrix;
        }
    }

    void SkeletonAnimationComponent::ComputeGlobalTransformRecursive(int index,
                                                                     std::vector<glm::mat4>& globals,
                                                                     std::vector<bool>& computed) {
        if (index < 0 || computed[index]) {
            return;
        }

        auto& joint = skeleton->joints[index];

        glm::mat4 local =
            glm::translate(glm::mat4(1.0f), joint.position) * glm::mat4_cast(joint.rotation) * glm::scale(glm::mat4(1.0f), joint.scale);

        if (joint.parentIndex >= 0) {
            ComputeGlobalTransformRecursive(joint.parentIndex, globals, computed);
            globals[index] = globals[joint.parentIndex] * local;
        } else {
            globals[index] = local;
        }

        computed[index] = true;
    }


    void SkeletonAnimationComponent::SetSkeleton(const std::shared_ptr<Skeleton>& pSkeleton) {
        skeleton = pSkeleton;
        
        if (skeleton) {
            skeleton->jointMatrices.resize(skeleton->joints.size(), glm::mat4(1.0f));
        }

        BuildTrackIndices();
    }

    void SkeletonAnimationComponent::SetClip(SkeletonAnimationClip* clip) {
        animationClip = clip;
        BuildTrackIndices();
    }

    void SkeletonAnimationComponent::RegisterClip(const std::string_view pName, const std::shared_ptr<SkeletonAnimationClip>& clip) {
        animationClips[std::string(pName)] = clip;
    }

    void SkeletonAnimationComponent::Play(const std::string_view pName, bool loop) {
        auto it = animationClips.find(std::string(pName));
        if (it != animationClips.end()) {
            animationClip = it->second.get();
            time          = 0.0f;
            playing       = true;
            looping       = loop;
            BuildTrackIndices();
        }
    }

    void SkeletonAnimationComponent::BuildTrackIndices() {
        if (!animationClip || !skeleton) {
            return;
        }
        for (auto& track : animationClip->tracks) {
            track.targetJointIndex = -1;
            for (size_t i = 0; i < skeleton->joints.size(); ++i) {
                if (skeleton->joints[i].name == track.targetJointName) {
                    track.targetJointIndex = static_cast<int>(i);
                    break;
                }
            }
        }
    }
} // namespace golias
