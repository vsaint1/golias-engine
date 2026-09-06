#include "scene/components/widget/dropdown_component.h"

#include "core/engine.h"
#include "font/font.h"
#include "graphics/texture_2d.h"
#include "scene/components/widget/canvas_component.h"
#include "scene/components/widget/rect_transform_component.h"
#include "scene/game_object.h"

namespace golias {

    constexpr const char* kDefaultArrowPath = "textures/UIArrowDown.png";

    bool DropdownComponent::LoadProperties(const Json& properties) {
        Component::LoadProperties(properties);

        if (properties.contains("options") && properties["options"].is_array()) {
            std::vector<String> options;

            for (const auto& option : properties["options"]) {
                if (option.is_string()) {
                    options.emplace_back(option.get<String>());
                }
            }

            SetOptions(options);
        }

        if (properties.contains("selected")) {
            mSelectedIndex = properties["selected"].get<int>();
            mSelectedIndex = glm::clamp(mSelectedIndex, 0, static_cast<int>(mOptions.size()) - 1);
        }

        if (properties.contains("font") && properties["font"].is_object()) {
            const Json& fontObj = properties["font"];

            mFontPath = fontObj.value("path", "");
            mFontSize = fontObj.value("size", 16);

            if (!mFontPath.empty()) {
                mFont = Engine::GetInstance().GetAssetManager().Load<Font>(mFontPath.c_str(), mFontSize);
            }
        }

        if (properties.contains("color")) {
            const Json& colorObj = properties["color"];
            const float r        = colorObj.value("r", mColor.r);
            const float g        = colorObj.value("g", mColor.g);
            const float b        = colorObj.value("b", mColor.b);
            const float a        = colorObj.value("a", mColor.a);

            mColor = glm::vec4(r, g, b, a);

            if (properties.contains("text")) {
                const Json& colorObj = properties["text"];
                const float r        = colorObj.value("r", mTextColor.r);
                const float g        = colorObj.value("g", mTextColor.g);
                const float b        = colorObj.value("b", mTextColor.b);
                const float a        = colorObj.value("a", mTextColor.a);

                mTextColor = glm::vec4(r, g, b, a);
            }
        }


        String arrowPath = kDefaultArrowPath;

        if (properties.contains("arrow") && properties["arrow"].is_object()) {
            arrowPath = properties["arrow"].value("path", kDefaultArrowPath);
        }

        if (!arrowPath.empty()) {
            mArrow = Engine::GetInstance().GetAssetManager().Load<Texture2D>(arrowPath.c_str());
        }

        return true;
    }

    void DropdownComponent::Update(float deltaTime) {
        UNUSED_PARAMETER(deltaTime);

        if (!mOpen) {
            return;
        }

        const InputManager& inputManager = Engine::GetInstance().GetInputManager();
        const glm::vec2 mousePos         = inputManager.GetMousePosition();

        mHoveredRow = RowAt(mousePos);

        if (inputManager.IsMouseButtonJustReleased(MouseButton::Left) && !HitTest(mousePos)) {
            SetOpen(false);
        }
    }

    void DropdownComponent::Render(CanvasComponent* canvas) {
        if (!canvas) {
            return;
        }

        glm::vec2 lowerLeft;
        glm::vec2 size;

        if (!GetHeaderRect(lowerLeft, size)) {
            return;
        }

        if (!mIsEnabled) {
            mCurrentColor = &mDisabledColor;
        }

        canvas->DrawQuad(lowerLeft, lowerLeft + size, *mCurrentColor);

        const float lineHeight = mFont ? static_cast<float>(mFont->GetLineHeight()) : 0.0f;
        const float headerTop  = lowerLeft.y + (size.y - lineHeight) * 0.5f;

        if (mFont && mSelectedIndex >= 0 && mSelectedIndex < static_cast<int>(mOptions.size())) {
            canvas->DrawText(mFont.get(), glm::vec2(lowerLeft.x + kHeaderPaddingX, headerTop), mOptions[mSelectedIndex], mTextColor);
        }

        if (mArrow) {
            const TextureDesc& desc = mArrow->GetDesc();
            const float aspect      = static_cast<float>(desc.Height) / static_cast<float>(desc.Width);

            const float arrowWidth  = size.y * 0.5f;
            const float arrowHeight = arrowWidth * aspect;

            const glm::vec2 arrowCenter(lowerLeft.x + size.x - kArrowInsetX, lowerLeft.y + size.y * 0.5f);
            const glm::vec2 arrowLowerLeft  = arrowCenter - glm::vec2(arrowWidth, arrowHeight) * 0.5f;
            const glm::vec2 arrowUpperRight = arrowCenter + glm::vec2(arrowWidth, arrowHeight) * 0.5f;

            const glm::vec2 uvLowerLeft(0.0f, mOpen ? 1.0f : 0.0f);
            const glm::vec2 uvUpperRight(1.0f, mOpen ? 0.0f : 1.0f);

            canvas->DrawQuad(arrowLowerLeft, arrowUpperRight, uvLowerLeft, uvUpperRight, mArrow.get(), mTextColor);
        }

        if (!mOpen || !mFont) {
            return;
        }

        const float rowHeight  = GetRowHeight();
        const float listTop    = lowerLeft.y + size.y + kListGap;
        const float listHeight = rowHeight * static_cast<float>(mOptions.size());

        canvas->DrawQuad(glm::vec2(lowerLeft.x, listTop), glm::vec2(lowerLeft.x + size.x, listTop + listHeight), mListColor);

        for (size_t i = 0; i < mOptions.size(); ++i) {
            const float rowY = listTop + static_cast<float>(i) * rowHeight;
            const glm::vec2 rowLowerLeft(lowerLeft.x, rowY);
            const glm::vec2 rowUpperRight(lowerLeft.x + size.x, rowY + rowHeight);

            if (static_cast<int>(i) == mHoveredRow) {
                canvas->DrawQuad(rowLowerLeft, rowUpperRight, mRowHoveredColor);
            } else if (static_cast<int>(i) == mSelectedIndex) {
                canvas->DrawQuad(rowLowerLeft, rowUpperRight, mSelectedRowColor);
            }

            const float rowTop = rowY + (rowHeight - lineHeight) * 0.5f;
            canvas->DrawText(mFont.get(), glm::vec2(lowerLeft.x + kHeaderPaddingX, rowTop), mOptions[i], mTextColor);
        }
    }

    bool DropdownComponent::HitTest(const glm::vec2& point) {
        if (!mIsEnabled) {
            return false;
        }

        glm::vec2 lowerLeft;
        glm::vec2 size;

        if (!GetHeaderRect(lowerLeft, size)) {
            return false;
        }

        const glm::vec2 p1 = lowerLeft;
        const glm::vec2 p2 = lowerLeft + size;

        if (point.x >= p1.x && point.x <= p2.x && point.y >= p1.y && point.y <= p2.y) {
            return true;
        }

        return RowAt(point) >= 0;
    }

    void DropdownComponent::OnPointerEnter() {
        mCurrentColor = &mHoveredColor;
    }

    void DropdownComponent::OnPointerExit() {
        mCurrentColor = &mColor;
    }

    void DropdownComponent::OnPointerUp() {
        mCurrentColor = &mHoveredColor;
    }

    void DropdownComponent::OnPointerDown() {
        mCurrentColor = &mPressedColor;
    }

    void DropdownComponent::OnClick() {
        if (!mOpen) {
            SetOpen(true);
            return;
        }

        const InputManager& inputManager = Engine::GetInstance().GetInputManager();
        const int row                    = RowAt(inputManager.GetMousePosition());

        if (row >= 0) {
            SetSelectedIndex(row);
        }

        SetOpen(false);
    }

    const std::vector<String>& DropdownComponent::GetOptions() const {
        return mOptions;
    }

    void DropdownComponent::SetOptions(const std::vector<String>& options) {
        mOptions = options;

        if (mSelectedIndex < 0 || mSelectedIndex >= static_cast<int>(mOptions.size())) {
            mSelectedIndex = mOptions.empty() ? 0 : static_cast<int>(mOptions.size()) - 1;
        }
    }

    void DropdownComponent::AddOption(const String& option) {
        mOptions.push_back(option);

        if (mSelectedIndex < 0 || mSelectedIndex >= static_cast<int>(mOptions.size())) {
            mSelectedIndex = static_cast<int>(mOptions.size()) - 1;
        }
    }

    void DropdownComponent::ClearOptions() {
        mOptions.clear();
        mSelectedIndex = -1;
    }

    bool DropdownComponent::RemoveOption(const String& option) {
        auto it = std::find(mOptions.begin(), mOptions.end(), option);

        if (it != mOptions.end()) {
            int index = static_cast<int>(std::distance(mOptions.begin(), it));
            RemoveOptionAt(index);
            return true;
        }

        return false;
    }

    void DropdownComponent::RemoveOptionAt(int index) {
        if (index < 0 || index >= static_cast<int>(mOptions.size())) {
            return;
        }

        mOptions.erase(mOptions.begin() + index);

        if (mSelectedIndex == index) {
            mSelectedIndex = -1;
        } else if (mSelectedIndex > index) {
            --mSelectedIndex;
        }
    }

    int DropdownComponent::GetSelectedIndex() const {
        return mSelectedIndex;
    }

    void DropdownComponent::SetSelectedIndex(int index, bool notify) {
        if (index < 0 || index >= static_cast<int>(mOptions.size()) || index == mSelectedIndex) {
            return;
        }

        mSelectedIndex = index;

        if (notify && onValueChanged) {
            onValueChanged(mSelectedIndex);
        }
    }

    const String& DropdownComponent::GetSelectedOption() const {
        static const String kEmptyString;

        if (mSelectedIndex < 0 || mSelectedIndex >= static_cast<int>(mOptions.size())) {
            return kEmptyString;
        }

        return mOptions[mSelectedIndex];
    }

    bool DropdownComponent::IsOpen() const {
        return mOpen;
    }

    void DropdownComponent::SetOpen(bool open) {
        if (mOpen == open) {
            return;
        }

        mOpen = open;

        if (!mOpen) {
            mHoveredRow = -1;
        }
    }

    bool DropdownComponent::GetHeaderRect(glm::vec2& lowerLeft, glm::vec2& size) const {
        RectTransformComponent* rectTransform = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rectTransform) {
            return false;
        }

        const glm::vec2 screenPos = rectTransform->GetScreenPosition();
        size                      = rectTransform->GetSize();
        lowerLeft                 = screenPos - rectTransform->GetPivot() * size;

        return true;
    }

    float DropdownComponent::GetRowHeight() const {
        const float lineHeight = mFont ? static_cast<float>(mFont->GetLineHeight()) : mFontSize;
        return lineHeight + kRowPadding * 2.0f;
    }

    int DropdownComponent::RowAt(const glm::vec2& point) const {
        if (!mOpen) {
            return -1;
        }

        glm::vec2 lowerLeft;
        glm::vec2 size;

        if (!GetHeaderRect(lowerLeft, size)) {
            return -1;
        }

        const float rowHeight = GetRowHeight();
        const float listTop   = lowerLeft.y + size.y + kListGap;

        if (point.x < lowerLeft.x || point.x > lowerLeft.x + size.x || point.y < listTop) {
            return -1;
        }

        const int row = static_cast<int>((point.y - listTop) / rowHeight);

        if (row < 0 || row >= static_cast<int>(mOptions.size())) {
            return -1;
        }

        return row;
    }

    bool DropdownComponent::IsTopmost() const {
        return mOpen;
    }

} // namespace golias
