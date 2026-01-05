#pragma once

#include "scene/component.h"
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

namespace golias {

    class Texture2D;

    class SpriteComponent2D : public Component {
        COMPONENT(SpriteComponent2D)
    public:
        void SetTexture(const std::shared_ptr<Texture2D>& tex);
        std::shared_ptr<Texture2D> GetTexture() const;

        void SetColor(const glm::vec4& value);
        glm::vec4 GetColor() const;

        void SetSize(const glm::vec2& value);
        glm::vec2 GetSize() const;

        void SetLowerLeftUV(const glm::vec2& uv);
        glm::vec2 GetLowerLeftUV() const;

        void SetUpperRightUV(const glm::vec2& uv);
        glm::vec2 GetUpperRightUV() const;

        void SetPivot(const glm::vec2& value);
        glm::vec2 GetPivot() const;

        void SetVisible(bool value);
        bool IsVisible() const;

        void Start() override;
        void Update(float deltaTime) override;
        void LoadProperties(const nlohmann::json& json) override;

    private:
        std::shared_ptr<Texture2D> texture;
        glm::vec4 color = glm::vec4(1.0f);
        glm::vec2 size  = glm::vec2(32.0f, 32.0f);
        glm::vec2 lowerLeftUV  = glm::vec2(0.0f, 0.0f);
        glm::vec2 upperRightUV = glm::vec2(1.0f, 1.0f);
        glm::vec2 pivot = glm::vec2(0.5f, 0.5f); // normalized (0.0 - 1.0)
        bool visible = true;
    };
} // namespace golias
