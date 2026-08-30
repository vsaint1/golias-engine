#pragma once

#include "widget_component.h"

namespace golias {

    class Texture2D;

    class CheckBoxComponent : public WidgetComponent {
        COMPONENT_DERIVED(CheckBoxComponent, WidgetComponent)
    public:
        CheckBoxComponent()  = default;
        ~CheckBoxComponent() = default;

        bool LoadProperties(const Json& properties) override;

        void Update(float deltaTime) override;

        void Render(CanvasComponent* canvas) override;

        bool HitTest(const glm::vec2& point) override;

        void OnPointerEnter() override;
        void OnPointerExit() override;

        void OnPointerUp() override;
        void OnPointerDown() override;

        void OnClick() override;

        bool IsChecked() const;
        void SetChecked(bool checked, bool notify = true);

        const glm::vec4& GetColor() const;
        void SetColor(const glm::vec4& color);

        std::function<void(bool)> onValueChanged;

    private:
        static constexpr float kCheckInsetFraction = 0.15f;

        glm::vec4 mColor  = glm::vec4(1.0f);
        glm::vec4 mHoveredColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        glm::vec4 mPressedColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);

        glm::vec4* mCurrentColor = &mColor;

        bool mChecked     = false;
        Ref<Texture2D> mCheckMark = nullptr;
    };

} // namespace golias