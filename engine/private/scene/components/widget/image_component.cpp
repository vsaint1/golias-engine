#include "scene/components/widget/image_component.h"

#include "core/engine.h"
#include "graphics/texture_2d.h"
#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {

    bool ImageComponent::LoadProperties(const Json& properties) {

        if (properties.contains("texture") && properties["texture"].is_object()) {
            const Json& textureJson  = properties["texture"];
            const String texturePath = textureJson.value("path", "");

            if (!texturePath.empty()) {
                mTexture = Engine::GetInstance().GetAssetManager().Load<Texture2D>(texturePath.c_str());
            }
        }

        if (properties.contains("color")) {
            const Json& colorObj = properties["color"];
            const float r        = colorObj.value("r", 1.0f);
            const float g        = colorObj.value("g", 1.0f);
            const float b        = colorObj.value("b", 1.0f);
            const float a        = colorObj.value("a", 1.0f);

            SetColor(glm::vec4(r, g, b, a));
        }

        return true;
    }

    void ImageComponent::Update(float deltaTime) {
    }

    void ImageComponent::Render(CanvasComponent* canvas) {
        if (!canvas) {
            return;
        }

        // NOTE: Asset fallback to error texture if the main texture is not loaded
        const Ref<Texture2D>& texture = mTexture ? mTexture : Engine::GetInstance().GetAssetManager().AcquireErrorTexture();
        if (!texture) {
            return;
        }

        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();

        glm::vec2 screenPos;
        glm::vec2 size;
        glm::vec2 pivot = glm::vec2(0.5f, 0.5f);

        if (rectTransform) {
            screenPos = rectTransform->GetScreenPosition();
            size      = rectTransform->GetSize();
            pivot     = rectTransform->GetPivot();
        } else {
            screenPos = GetOwner()->GetPosition2D();
            size      = glm::vec2(static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight()));
        }

        const glm::vec2 lowerLeft = screenPos - pivot * size;

        canvas->DrawQuad(lowerLeft, lowerLeft + size, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), texture.get(), mColor);
    }

    const Ref<Texture2D>& ImageComponent::GetTexture() const {
        return mTexture;
    }

    void ImageComponent::SetTexture(const Ref<Texture2D>& texture) {
        mTexture = texture;
    }

    const glm::vec4& ImageComponent::GetColor() const {
        return mColor;
    }

    void ImageComponent::SetColor(const glm::vec4& color) {
        mColor = color;
    }

} // namespace golias
