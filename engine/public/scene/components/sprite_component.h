#pragma once

#include "component.h"

namespace golias {

    class Texture2D;

    class SpriteComponent : public Component {
        COMPONENT(SpriteComponent)
    public:
        bool LoadProperties(const Json& properties);

        void Update(float deltaTime) override;

        Ref<Texture2D> GetTexture() const;
        void SetTexture(const Ref<Texture2D>& texture);

        glm::vec2 GetLowerLeftUV() const;
        void SetLowerLeftUV(const glm::vec2& uv);

        glm::vec2 GetUpperRightUV() const;
        void SetUpperRightUV(const glm::vec2& uv);

        glm::vec2 GetPivot() const;
        void SetPivot(const glm::vec2& pivot);

        glm::vec2 GetSize() const;
        void SetSize(const glm::vec2& size);

        glm::vec4 GetColor() const;
        void SetColor(const glm::vec4& color);

        bool IsVisible() const;
        void SetVisible(bool visible);

    private:
        Ref<Texture2D> mTexture = nullptr;

        glm::vec2 mLowerLeftUV  = glm::vec2(0.0f);
        glm::vec2 mUpperRightUV = glm::vec2(1.0f);

        glm::vec2 mPivot = glm::vec2(0.5f);

        glm::vec2 mSize = glm::vec2(100.0f);

        glm::vec4 mColor = glm::vec4(1.0f);

        bool mVisible = true;
    };
} // namespace golias
