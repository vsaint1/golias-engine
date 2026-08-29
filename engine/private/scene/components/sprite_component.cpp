#include "scene/components/sprite_component.h"

#include "core/engine.h"
#include "graphics/texture_2d.h"
#include "scene/game_object.h"

namespace golias {

    bool SpriteComponent::LoadProperties(const Json& properties) {

        const String texturePath = properties.value("texture", "");

        if (!texturePath.empty()) {
            mTexture = Engine::GetInstance().GetAssetManager().Load<Texture2D>(texturePath.c_str());
        }

        if (properties.contains("color")) {
            const Json& colorJson = properties["color"];
            glm::vec4 color =
                glm::vec4(colorJson.value("r", 1.0f), colorJson.value("g", 1.0f), colorJson.value("b", 1.0f), colorJson.value("a", 1.0f));
            mColor = color;
        }

        if (properties.contains("visible")) {
            mVisible = properties["visible"].get<bool>();
        }

        if (properties.contains("lower_left_uv")) {
            const Json& lowerLeftUVJson = properties["lower_left_uv"];
            glm::vec2 lowerLeftUV       = glm::vec2(lowerLeftUVJson.value("x", 0.0f), lowerLeftUVJson.value("y", 0.0f));
            mLowerLeftUV                = lowerLeftUV;
        }

        if (properties.contains("upper_right_uv")) {
            const Json& upperRightUVJson = properties["upper_right_uv"];
            glm::vec2 upperRightUV       = glm::vec2(upperRightUVJson.value("x", 1.0f), upperRightUVJson.value("y", 1.0f));
            mUpperRightUV                = upperRightUV;
        }

        if (properties.contains("pivot")) {
            const Json& pivotJson = properties["pivot"];
            glm::vec2 pivot       = glm::vec2(pivotJson.value("x", 0.5f), pivotJson.value("y", 0.5f));
            mPivot                = pivot;
        }

        if (properties.contains("size")) {
            const Json& sizeJson = properties["size"];
            glm::vec2 size       = glm::vec2(sizeJson.value("x", 100.0f), sizeJson.value("y", 100.0f));
            mSize                = size;
        }

        return true;
    }

    void SpriteComponent::Update(float deltaTime) {
        if (!mTexture || !mVisible) {
            return;
        }

        if (!GetOwner()) {
            return;
        }

        RenderCommand2D command = {
            .Texture      = mTexture.get(),
            .Color        = mColor,
            .Model        = GetOwner()->GetWorldTransform2D(),
            .Size         = mSize,
            .Pivot        = mPivot,
            .LowerLeftUV  = mLowerLeftUV,
            .UpperRightUV = mUpperRightUV,
        };

        Engine::GetInstance().GetCommandQueue().Submit(command);
    }

    Ref<Texture2D> SpriteComponent::GetTexture() const {
        return mTexture;
    }

    void SpriteComponent::SetTexture(const Ref<Texture2D>& texture) {
        mTexture = texture;
    }

    glm::vec2 SpriteComponent::GetLowerLeftUV() const {
        return mLowerLeftUV;
    }

    void SpriteComponent::SetLowerLeftUV(const glm::vec2& uv) {
        mLowerLeftUV = uv;
    }

    glm::vec2 SpriteComponent::GetUpperRightUV() const {
        return mUpperRightUV;
    }

    void SpriteComponent::SetUpperRightUV(const glm::vec2& uv) {
        mUpperRightUV = uv;
    }

    glm::vec2 SpriteComponent::GetPivot() const {
        return mPivot;
    }

    void SpriteComponent::SetPivot(const glm::vec2& pivot) {
        mPivot = pivot;
    }

    glm::vec2 SpriteComponent::GetSize() const {
        return mSize;
    }

    void SpriteComponent::SetSize(const glm::vec2& size) {
        mSize = size;
    }

    glm::vec4 SpriteComponent::GetColor() const {
        return mColor;
    }

    void SpriteComponent::SetColor(const glm::vec4& color) {
        mColor = color;
    }

    bool SpriteComponent::IsVisible() const {
        return mVisible;
    }

    void SpriteComponent::SetVisible(bool visible) {
        mVisible = visible;
    }

} // namespace golias
