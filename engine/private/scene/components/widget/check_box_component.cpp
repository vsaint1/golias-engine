#include "scene/components/widget/check_box_component.h"

#include "core/engine.h"
#include "graphics/texture_2d.h"
#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {

    namespace {
        constexpr const char* kDefaultCheckMarkPath = "textures/UICheckMark.png";
    } // namespace

    bool CheckBoxComponent::LoadProperties(const Json& properties) {
        Component::LoadProperties(properties);

        if (properties.contains("color")) {
            const Json& colorObj = properties["color"];
            const float r        = colorObj.value("r", 1.0f);
            const float g        = colorObj.value("g", 1.0f);
            const float b        = colorObj.value("b", 1.0f);
            const float a        = colorObj.value("a", 1.0f);

            SetColor(glm::vec4(r, g, b, a));
        }

        if (properties.contains("checked")) {
            mChecked = properties["checked"].get<bool>();
        }

        if (properties.contains("check_mark") && properties["check_mark"].is_object()) {
            const Json& checkMarkJson  = properties["check_mark"];
            const String checkMarkPath = checkMarkJson.value("path", kDefaultCheckMarkPath);

            if (!checkMarkPath.empty()) {
                mCheckMark = Engine::GetInstance().GetAssetManager().Load<Texture2D>(checkMarkPath.c_str());
            }
        }

        if (!mCheckMark) {
            mCheckMark = Engine::GetInstance().GetAssetManager().Load<Texture2D>(kDefaultCheckMarkPath);
        }


        return true;
    }

    void CheckBoxComponent::Update(float deltaTime) {
    }

    void CheckBoxComponent::Render(CanvasComponent* canvas) {
        if (!canvas) {
            return;
        }

        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return;
        }

        const glm::vec2 screenPos = rectTransform->GetScreenPosition();
        const glm::vec2 size      = rectTransform->GetSize();
        const glm::vec2 lowerLeft = screenPos - rectTransform->GetPivot() * size;

        if (!mIsEnabled) {
            mCurrentColor = &mDisabledColor;
        }

        canvas->DrawQuad(lowerLeft, lowerLeft + size, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), nullptr, *mCurrentColor);

        if (mChecked && mCheckMark) {
            const glm::vec2 inset          = size * kCheckInsetFraction;
            const glm::vec2 markLowerLeft  = lowerLeft + inset;
            const glm::vec2 markUpperRight = lowerLeft + size - inset;

            canvas->DrawQuad(markLowerLeft,
                             markUpperRight,
                             glm::vec2(0.0f, 0.0f),
                             glm::vec2(1.0f, 1.0f),
                             mCheckMark.get(),
                             glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }
    }

    bool CheckBoxComponent::HitTest(const glm::vec2& point) {

        if (!mIsEnabled) {
            return false;
        }

        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return false;
        }

        const glm::vec2 screenPos = rectTransform->GetScreenPosition();
        const glm::vec2 size      = rectTransform->GetSize();
        const glm::vec2 p1        = screenPos - rectTransform->GetPivot() * size;
        const glm::vec2 p2        = p1 + size;

        return (point.x >= p1.x && point.x <= p2.x && point.y >= p1.y && point.y <= p2.y);
    }

    void CheckBoxComponent::OnPointerEnter() {
        mCurrentColor = &mHoveredColor;
    }

    void CheckBoxComponent::OnPointerExit() {
        mCurrentColor = &mColor;
    }

    void CheckBoxComponent::OnPointerUp() {
        mCurrentColor = &mHoveredColor;
    }

    void CheckBoxComponent::OnPointerDown() {
        mCurrentColor = &mPressedColor;
    }

    void CheckBoxComponent::OnClick() {
        SetChecked(!mChecked);
    }

    bool CheckBoxComponent::IsChecked() const {
        return mChecked;
    }

    void CheckBoxComponent::SetChecked(bool checked, bool notify) {
        if (mChecked == checked) {
            return;
        }

        mChecked      = checked;
        mCurrentColor = &mColor;

        if (notify && onValueChanged) {
            onValueChanged(mChecked);
        }
    }

    const glm::vec4& CheckBoxComponent::GetColor() const {
        return mColor;
    }

    void CheckBoxComponent::SetColor(const glm::vec4& color) {
        mColor        = color;
        mCurrentColor = &mColor;
    }

} // namespace golias
