#include "scene/components/sprite_component.h"

#include "core/engine.h"
#include "graphics/texture_2d.h"
#include "scene/game_object.h"

namespace golias {

    bool SpriteComponent::LoadProperties(const Json& properties) {


        if (properties.contains("texture") && properties["texture"].is_object()) {
            const Json& textureJson = properties["texture"];
            const String texturePath = textureJson.value("path", "");

            if (!texturePath.empty()) {
                mTexture = Engine::GetInstance().GetAssetManager().Load<Texture2D>(texturePath.c_str());
            }

            if (textureJson.contains("visible")) {
                mVisible = textureJson["visible"].get<bool>();
            }

            if (textureJson.contains("lower_left_uv")) {
                const Json& lowerLeftUVJson = textureJson["lower_left_uv"];
                const float x = lowerLeftUVJson.value("x", 0.0f);
                const float y = lowerLeftUVJson.value("y", 0.0f);

                mLowerLeftUV                = glm::vec2(x, y);
            }

            if (textureJson.contains("upper_right_uv")) {
                const Json& upperRightUVJson = textureJson["upper_right_uv"];
                const float x = upperRightUVJson.value("x", 1.0f);
                const float y = upperRightUVJson.value("y", 1.0f);
                
                mUpperRightUV                = glm::vec2(x, y);
            }

            if (textureJson.contains("pivot")) {
                
                const Json& pivotJson = textureJson["pivot"];
                const float x = pivotJson.value("x", 0.5f);
                const float y = pivotJson.value("y", 0.5f);

                mPivot                = glm::vec2(x, y);
            }

            if (textureJson.contains("size")) {
                const Json& sizeJson = textureJson["size"];
                const float x = sizeJson.value("x", 100.0f);
                const float y = sizeJson.value("y", 100.0f);

                mSize                = glm::vec2(x, y);
            }

            if (properties.contains("description") && properties["description"].is_object()) {
            }
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
