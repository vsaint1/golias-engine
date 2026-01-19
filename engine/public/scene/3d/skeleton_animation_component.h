#pragma once
#include "scene/component.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace golias {

    struct KeyFrameVec3;
    struct KeyFrameQuat;

    struct SkeletonJoint {
        std::string name;
        int parentIndex = -1;
        glm::mat4 inverseBindMatrix = glm::mat4(1.0f);
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
    };

    struct SkeletonAnimationTrack {
        std::string targetJointName;
        int targetJointIndex = -1;
        std::vector<KeyFrameVec3> positions;
        std::vector<KeyFrameQuat> rotations;
        std::vector<KeyFrameVec3> scales;
    };

    struct SkeletonAnimationClip {
        std::string name;
        float duration = 0.0f;
        bool looping = true;
        std::vector<SkeletonAnimationTrack> tracks;
    };

    struct Skeleton {
        std::string name;
        glm::mat4 globalInverseTransform = glm::mat4(1.0f); 
        std::vector<SkeletonJoint> joints;
        std::vector<glm::mat4> jointMatrices;
    };

    class SkeletonAnimationComponent : public Component {
        COMPONENT(SkeletonAnimationComponent)
    public:

        void Start() override;
        void Update(float deltaTime) override;

        void SetSkeleton(const std::shared_ptr<Skeleton>& pSkeleton);
        void SetClip(SkeletonAnimationClip* clip);
        void RegisterClip(const std::string_view pName, const std::shared_ptr<SkeletonAnimationClip>& clip);
        void Play(const std::string_view pName, bool loop = true);

        bool IsPlaying() const;

        Skeleton* GetSkeleton() const { return skeleton.get(); }
        const std::vector<glm::mat4>& GetJointMatrices() const;

        std::unordered_map<std::string, std::shared_ptr<SkeletonAnimationClip>>& GetRegisteredClips();

        void LoadProperties(const nlohmann::json& json) override {
        }

    private:
        void BuildTrackIndices();
        void UpdateJointMatrices();

    private:
        std::shared_ptr<Skeleton> skeleton;
        SkeletonAnimationClip* animationClip = nullptr;
        float time = 0.0f;
        bool looping = true;
        bool playing = false;

        std::unordered_map<std::string, std::shared_ptr<SkeletonAnimationClip>> animationClips;
        std::vector<int> trackToJointIndex;

        void ComputeGlobalTransformRecursive(int index, std::vector<glm::mat4>& globals, std::vector<bool>& computed);
    };

} // namespace golias
