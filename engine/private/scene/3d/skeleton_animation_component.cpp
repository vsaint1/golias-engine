#include "scene/3d/skeleton_animation_component.h"
#include "scene/3d/animation_component.h"
#include "scene/game_object.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/spdlog.h>

namespace golias {

    void SkeletonAnimationComponent::Start() {
        BuildTrackIndices();

        if (skeleton) {
            skeleton->jointMatrices.resize(skeleton->joints.size(), glm::mat4(1.0f));
        }
    }

    void SkeletonAnimationComponent::Update(float deltaTime) {
        if (!animationClip || !skeleton) {
            return;
        }

        if (!playing) {
            return;
        }

        time += deltaTime;

        if (time > animationClip->duration) {
            if (looping) {
                time = std::fmod(time, animationClip->duration);
            } else {
                time = animationClip->duration;
                playing = false;
                return;
            }
        }

        for (size_t i = 0; i < animationClip->tracks.size(); ++i) {
            auto& track = animationClip->tracks[i];
            if (track.targetJointIndex < 0 || track.targetJointIndex >= skeleton->joints.size()) {
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

    void SkeletonAnimationComponent::SetSkeleton(std::shared_ptr<Skeleton> skel) {
        skeleton = skel;
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
        auto it = animationClips.find(pName.data());
        if (it == animationClips.end()) {
            spdlog::warn("SkeletonAnimationComponent::Play Skeleton animation clip not found: {}. Available clips: {}", pName, animationClips.size());
            for (const auto& [name, clip] : animationClips) {
                spdlog::info("  - {}", name);
            }
            return;
        }

        if (animationClip && animationClip->name == pName && playing) {
            spdlog::info("SkeletonAnimationComponent::Play Already playing skeleton animation: {}", pName);
            return;
        }

        SetClip(it->second.get());
        time = 0.0f;
        playing = true;
        looping = loop;
        spdlog::info("SkeletonAnimationComponent::Play Started skeleton animation: {} with {} tracks, duration: {:.2f}s", 
                     pName, animationClip->tracks.size(), animationClip->duration);
    }

    bool SkeletonAnimationComponent::IsPlaying() const {
        return playing;
    }

    const std::vector<glm::mat4>& SkeletonAnimationComponent::GetJointMatrices() const {
        return skeleton->jointMatrices;
    }

    std::unordered_map<std::string, std::shared_ptr<SkeletonAnimationClip>>& SkeletonAnimationComponent::GetRegisteredClips() {
        return animationClips;
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

    void SkeletonAnimationComponent::UpdateJointMatrices() {
        if (!skeleton) {
            return;
        }

        std::vector<glm::mat4> globalTransforms(skeleton->joints.size(), glm::mat4(1.0f));

        for (size_t i = 0; i < skeleton->joints.size(); ++i) {
            auto& joint = skeleton->joints[i];

            glm::mat4 translation = glm::translate(glm::mat4(1.0f), joint.position);
            glm::mat4 rotation = glm::mat4_cast(joint.rotation);
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), joint.scale);
            glm::mat4 localTransform = translation * rotation * scale;

            if (joint.parentIndex >= 0 && joint.parentIndex < static_cast<int>(globalTransforms.size())) {
                globalTransforms[i] = globalTransforms[joint.parentIndex] * localTransform;
            } else {
                globalTransforms[i] = localTransform;
            }
        }

        for (size_t i = 0; i < skeleton->joints.size(); ++i) {
            skeleton->jointMatrices[i] = globalTransforms[i] * skeleton->joints[i].inverseBindMatrix;
        }
    }


} // namespace golias
