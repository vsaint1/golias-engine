#include "scene/3d/animation_component.h"

#include "scene/game_object.h"

namespace golias {

    void AnimationComponent::Start() {
        BuildBindings();
    }

    void AnimationComponent::Update(float deltaTime) {
        if (!animation_clip) {
            return;
        }

        if (!playing) {
            return;
        }

        time += deltaTime;

        if (time > animation_clip->duration) {
            if (looping) {
                time = std::fmod(time, animation_clip->duration);
            } else {
                time    = animation_clip->duration; 
                playing = false;
                return;
            }
        }

        for (auto& binding : bindings) {
            auto& obj          = binding.first;
            auto& trackIndices = binding.second->trackIndices;

            for (auto i : trackIndices) {
                auto& track = animation_clip->tracks[i];

                if (!track.positions.empty()) {
                    auto pos = Interpolate(track.positions, time);
                    obj->SetPosition(pos);
                }

                if (!track.rotations.empty()) {
                    auto rot = Interpolate(track.rotations, time);
                    obj->SetRotation(rot);
                }
                
                if (!track.scales.empty()) {
                    auto scale = Interpolate(track.scales, time);
                    obj->SetScale(scale);
                }
            }
        }
    }

    void AnimationComponent::SetClip(AnimationClip* clip) {
        animation_clip = clip;
        // BuildBindings();
    }

    void AnimationComponent::RegisterClip(const std::string& name, const std::shared_ptr<AnimationClip>& clip) {
        animation_clips[name] = clip;
    }

    void AnimationComponent::Play(const std::string& name, bool loop) {
        if (animation_clip && animation_clip->name == name) {
            time    = 0.0f;
            playing = true;
            looping = loop;
        } else {
            auto it = animation_clips.find(name);
            if (it != animation_clips.end()) {
                SetClip(it->second.get());
                time    = 0.0f;
                playing = true;
                looping = loop;
            }
        }
    }

    bool AnimationComponent::IsPlaying() const {
        return playing;
    }

    void AnimationComponent::BuildBindings() {
        bindings.clear();
        if (!animation_clip) {
            return;
        }

        for (size_t i = 0; i < animation_clip->tracks.size(); ++i) {
            auto& track       = animation_clip->tracks[i];
            auto targetObject = GetOwner()->FindChildByName(track.targetName);
            if (targetObject) {
                auto it = bindings.find(targetObject);
                if (it != bindings.end()) {
                    it->second->trackIndices.push_back(i);
                } else {
                    auto binding    = std::make_unique<ObjectBinding>();
                    binding->object = targetObject;
                    binding->trackIndices.push_back(i);
                    bindings.emplace(targetObject, std::move(binding));
                }
            }
        }
    }

    glm::vec3 AnimationComponent::Interpolate(const std::vector<KeyFrameVec3>& keys, float time) {
        if (keys.empty()) {
            return glm::vec3(0.0f);
        }

        if (keys.size() == 1) {
            return keys[0].value;
        }

        if (time <= keys.front().time) {
            return keys.front().value;
        }

        if (time >= keys.back().time) {
            return keys.back().value;
        }

        size_t i0 = 0;
        size_t i1 = 0;

        for (size_t i = 1; i < keys.size(); ++i) {
            if (time <= keys[i].time) {
                i1 = i;
                i0 = i - 1;
                break;
            }
        }

        float deltaTime = keys[i1].time - keys[i0].time;
        if (deltaTime > 0.0f) {
            float k = (time - keys[i0].time) / deltaTime;
            return glm::mix(keys[i0].value, keys[i1].value, k);
        }

        return keys[i0].value;
    }

    glm::quat AnimationComponent::Interpolate(const std::vector<KeyFrameQuat>& keys, float time) {
        if (keys.empty()) {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        if (keys.size() == 1) {
            return keys[0].value;
        }

        if (time <= keys.front().time) {
            return keys.front().value;
        }

        if (time >= keys.back().time) {
            return keys.back().value;
        }

        size_t i0 = 0;
        size_t i1 = 0;

        for (size_t i = 1; i < keys.size(); ++i) {
            if (time <= keys[i].time) {
                i1 = i;
                i0 = i - 1;
                break;
            }
        }

        float deltaTime = keys[i1].time - keys[i0].time;
        if (deltaTime > 0.0f) {
            float k = (time - keys[i0].time) / deltaTime;
            return glm::slerp(keys[i0].value, keys[i1].value, k);
        }

        return keys[i0].value;
    }
} // namespace golias
