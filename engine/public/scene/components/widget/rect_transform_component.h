#pragma once

#include "scene/components/component.h"

namespace golias {

    class RectTransformComponent : public Component {
        COMPONENT(RectTransformComponent)
    public:
        RectTransformComponent()  = default;
        ~RectTransformComponent() = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        glm::vec2 GetSize() const;
        void SetSize(const glm::vec2& size);

        glm::vec2 GetAnchorPoint() const;
        void SetAnchorPoint(const glm::vec2& anchorPoint);

        glm::vec2 GetPivot() const;
        void SetPivot(const glm::vec2& pivot);

        glm::vec2 GetScreenPosition() const;
    private:
        glm::vec2 mSize = glm::vec2(1.0f, 1.0f);

        glm::vec2 mAnchorPoint = glm::vec2(0.5f, 0.5f);
        glm::vec2 mPivot = glm::vec2(0.5f, 0.5f);
    };


} // namespace golias
