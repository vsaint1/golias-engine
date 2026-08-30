#pragma once

#include "widget_component.h"

namespace golias {

    class Texture2D;

    class ImageComponent : public WidgetComponent {
        COMPONENT_DERIVED(ImageComponent, WidgetComponent)
    public:
        ImageComponent()  = default;
        ~ImageComponent() = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        void Render(CanvasComponent* canvas) override;

        const Ref<Texture2D>& GetTexture() const;
        void SetTexture(const Ref<Texture2D>& texture);

        const glm::vec4& GetColor() const;
        void SetColor(const glm::vec4& color);

    private:
        Ref<Texture2D> mTexture = nullptr;

        glm::vec4 mColor = glm::vec4(1.0f);
    };

} // namespace golias
