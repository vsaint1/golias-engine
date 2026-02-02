#pragma once

#include "scene/ui/ui_base.h"

#include <glm/vec2.hpp>

namespace golias {
    
    class RectTransformComponent : public Component {
        COMPONENT(RectTransformComponent)

    public:
        RectTransformComponent();
        virtual ~RectTransformComponent() = default;

        glm::vec2 GetSize() const;
        void SetSize(const glm::vec2& size);

        glm::vec2 GetAnchor() const;
        void SetAnchor(const glm::vec2& anchor);

        glm::vec2 GetPivot() const;
        void SetPivot(const glm::vec2& pivot);

        void Start();
        void Update(float deltaTime);
        void LoadProperties(const nlohmann::json& json);

        glm::vec2 GetScreenPosition() const;

    private:
        glm::vec2 mSize   = glm::vec2(100.0f, 100.0f);
        glm::vec2 mAnchor = glm::vec2(0.0f, 0.0f);
        glm::vec2 mPivot  = glm::vec2(0.5f, 0.5f);
    };
} // namespace golias
