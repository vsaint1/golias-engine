#include "scene/ui/dropdown_component.h"

#include "core/engine.h"
#include "core/input/input_manager.h"
#include "font/font.h"
#include "scene/game_object.h"
#include "scene/ui/canvas_component.h"
#include "scene/ui/rect_transform_component.h"

namespace golias {

    const std::string DropdownWidgetComponent::emptyOption = "";

    void DropdownWidgetComponent::AddOption(const std::string& option) {
        options.push_back(option);
    }

    void DropdownWidgetComponent::ClearOptions() {
        options.clear();
        selectedIndex = 0;
    }

    void DropdownWidgetComponent::SetOptions(const std::vector<std::string>& opts) {
        options = opts;
        if (selectedIndex >= static_cast<int>(options.size())) {
            selectedIndex = options.empty() ? 0 : static_cast<int>(options.size()) - 1;
        }
    }

    const std::vector<std::string>& DropdownWidgetComponent::GetOptions() const {
        return options;
    }

    void DropdownWidgetComponent::SetValue(int index) {
        int oldIndex = selectedIndex;
        selectedIndex = glm::clamp(index, 0, static_cast<int>(options.size()) - 1);
        
        if (selectedIndex != oldIndex && OnValueChanged) {
            OnValueChanged(selectedIndex, GetSelectedText());
        }
    }

    int DropdownWidgetComponent::GetValue() const {
        return selectedIndex;
    }

    const std::string& DropdownWidgetComponent::GetSelectedText() const {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(options.size())) {
            return options[selectedIndex];
        }
        return emptyOption;
    }

    void DropdownWidgetComponent::SetInteractable(bool val) {
        interactable = val;
        if (!interactable) {
            Collapse();
        }
    }

    bool DropdownWidgetComponent::IsInteractable() const {
        return interactable;
    }

    void DropdownWidgetComponent::SetBackgroundColor(const glm::vec4& color) {
        backgroundColor = color;
    }

    glm::vec4 DropdownWidgetComponent::GetBackgroundColor() const {
        return backgroundColor;
    }

    void DropdownWidgetComponent::SetHighlightColor(const glm::vec4& color) {
        highlightColor = color;
    }

    glm::vec4 DropdownWidgetComponent::GetHighlightColor() const {
        return highlightColor;
    }

    void DropdownWidgetComponent::SetTextColor(const glm::vec4& color) {
        textColor = color;
    }

    glm::vec4 DropdownWidgetComponent::GetTextColor() const {
        return textColor;
    }

    void DropdownWidgetComponent::SetDropdownBackgroundColor(const glm::vec4& color) {
        dropdownBackgroundColor = color;
    }

    glm::vec4 DropdownWidgetComponent::GetDropdownBackgroundColor() const {
        return dropdownBackgroundColor;
    }

    void DropdownWidgetComponent::SetItemHeight(float height) {
        itemHeight = height;
    }

    float DropdownWidgetComponent::GetItemHeight() const {
        return itemHeight;
    }

    void DropdownWidgetComponent::SetFont(const std::shared_ptr<Font>& pFont) {
        font = pFont;
    }

    const std::shared_ptr<Font>& DropdownWidgetComponent::GetFont() const {
        return font;
    }

    void DropdownWidgetComponent::SetFont(const std::string_view path, int size) {
        font = Engine::GetInstance().GetAssetManager().EnsureFont(path, size);
        fontSize = size;
    }

    void DropdownWidgetComponent::SetFontSize(int size) {
        fontSize = size;
        if (font) {
            font->SetSize(size);
        }
    }

    int DropdownWidgetComponent::GetFontSize() const {
        return fontSize;
    }

    bool DropdownWidgetComponent::IsExpanded() const {
        return expanded;
    }

    void DropdownWidgetComponent::Expand() {
        if (interactable && !options.empty()) {
            expanded = true;
        }
    }

    void DropdownWidgetComponent::Collapse() {
        expanded = false;
        hoveredIndex = -1;
    }

    void DropdownWidgetComponent::Start() {
        // Only set default font if no font has been set yet
        if (!font) {
            font = Engine::GetInstance().GetAssetManager().EnsureFont("fonts/NotoSans.ttf", fontSize);
        }
    }

    void DropdownWidgetComponent::Update(float deltaTime) {
        if (!expanded || !interactable) {
            return;
        }

        auto& input = Engine::GetInstance().GetInputManager();
        glm::vec2 mousePos = input.GetMousePosition();
        
        auto& renderer = Engine::GetInstance().GetSceneRenderer();
        const auto& viewport = renderer.GetRenderingDevice()->GetViewport();
        mousePos.y = viewport.height - mousePos.y;

        hoveredIndex = GetItemAtPosition(mousePos);
    }

    void DropdownWidgetComponent::Draw(CanvasComponent* pCanvas) {
        if (!pCanvas) {
            return;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return;
        }

        auto pos = rt->GetScreenPosition();
        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;

        // Draw main button background
        glm::vec4 bgColor = backgroundColor;
        if (!interactable) {
            bgColor *= 0.5f;
            bgColor.a = backgroundColor.a;
        }

        pCanvas->DrawTexture2D(
            glm::vec3(pos, 0.0f),
            glm::vec3(pos.x + scaledSize.x, pos.y + scaledSize.y, 0.0f),
            glm::vec2(0.0f),
            glm::vec2(1.0f),
            nullptr,
            bgColor);

        // Draw dropdown arrow indicator (simple triangle representation using a small square)
        float arrowSize = 10.0f * uiScale;
        float arrowX = pos.x + scaledSize.x - arrowSize - 10.0f * uiScale;
        float arrowY = pos.y + (scaledSize.y - arrowSize) * 0.5f;
        
        pCanvas->DrawTexture2D(
            glm::vec3(arrowX, arrowY, 0.01f),
            glm::vec3(arrowX + arrowSize, arrowY + arrowSize, 0.01f),
            glm::vec2(0.0f),
            glm::vec2(1.0f),
            nullptr,
            textColor);

        // Draw expanded dropdown list
        if (expanded && !options.empty()) {
            float scaledItemHeight = itemHeight * uiScale;
            float dropdownHeight = scaledItemHeight * options.size();
            
            // Draw dropdown background (below the main button)
            glm::vec2 dropdownPos(pos.x, pos.y - dropdownHeight);
            
            pCanvas->DrawTexture2D(
                glm::vec3(dropdownPos.x, dropdownPos.y, 0.05f),
                glm::vec3(dropdownPos.x + scaledSize.x, pos.y, 0.05f),
                glm::vec2(0.0f),
                glm::vec2(1.0f),
                nullptr,
                dropdownBackgroundColor);

            // Draw each option
            for (size_t i = 0; i < options.size(); ++i) {
                float itemY = pos.y - scaledItemHeight * (i + 1);
                
                // Highlight hovered item
                if (static_cast<int>(i) == hoveredIndex) {
                    pCanvas->DrawTexture2D(
                        glm::vec3(dropdownPos.x, itemY, 0.06f),
                        glm::vec3(dropdownPos.x + scaledSize.x, itemY + scaledItemHeight, 0.06f),
                        glm::vec2(0.0f),
                        glm::vec2(1.0f),
                        nullptr,
                        highlightColor);
                }
                
                // Highlight selected item
                if (static_cast<int>(i) == selectedIndex) {
                    pCanvas->DrawTexture2D(
                        glm::vec3(dropdownPos.x + 2.0f * uiScale, itemY + 2.0f * uiScale, 0.07f),
                        glm::vec3(dropdownPos.x + 4.0f * uiScale, itemY + scaledItemHeight - 2.0f * uiScale, 0.07f),
                        glm::vec2(0.0f),
                        glm::vec2(1.0f),
                        nullptr,
                        glm::vec4(0.4f, 0.6f, 1.0f, 1.0f));
                }
            }
        }

        // Text drawing would be handled by a TextWidgetComponent child
        // or by direct font rendering here if needed
    }

    void DropdownWidgetComponent::LoadProperties(const nlohmann::json& json) {
        if (json.contains("options") && json["options"].is_array()) {
            options.clear();
            for (const auto& opt : json["options"]) {
                options.push_back(opt.get<std::string>());
            }
        }

        selectedIndex = json.value("selectedIndex", 0);
        interactable = json.value("interactable", true);
        itemHeight = json.value("itemHeight", 30.0f);
        fontSize = json.value("fontSize", 14);

        if (json.contains("backgroundColor")) {
            const auto& col = json["backgroundColor"];
            backgroundColor = {col.value("r", 0.25f), col.value("g", 0.25f), col.value("b", 0.25f), col.value("a", 1.0f)};
        }

        if (json.contains("highlightColor")) {
            const auto& col = json["highlightColor"];
            highlightColor = {col.value("r", 0.35f), col.value("g", 0.35f), col.value("b", 0.35f), col.value("a", 1.0f)};
        }

        if (json.contains("textColor")) {
            const auto& col = json["textColor"];
            textColor = {col.value("r", 1.0f), col.value("g", 1.0f), col.value("b", 1.0f), col.value("a", 1.0f)};
        }

        if (json.contains("dropdownBackgroundColor")) {
            const auto& col = json["dropdownBackgroundColor"];
            dropdownBackgroundColor = {col.value("r", 0.2f), col.value("g", 0.2f), col.value("b", 0.2f), col.value("a", 1.0f)};
        }
    }

    bool DropdownWidgetComponent::HitTest(const glm::vec2& point) const {
        if (!interactable) {
            return false;
        }

        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt) {
            return false;
        }

        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        auto p1 = rt->GetScreenPosition();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;
        auto p2 = p1 + scaledSize;

        // Check main button
        if ((point.x >= p1.x) && (point.x <= p2.x) && (point.y >= p1.y) && (point.y <= p2.y)) {
            return true;
        }

        // Check expanded dropdown
        if (expanded && !options.empty()) {
            float scaledItemHeight = itemHeight * uiScale;
            float dropdownHeight = scaledItemHeight * options.size();
            
            glm::vec2 dropdownP1(p1.x, p1.y - dropdownHeight);
            glm::vec2 dropdownP2(p2.x, p1.y);
            
            if ((point.x >= dropdownP1.x) && (point.x <= dropdownP2.x) &&
                (point.y >= dropdownP1.y) && (point.y <= dropdownP2.y)) {
                return true;
            }
        }

        return false;
    }

    void DropdownWidgetComponent::OnPointerDown() {
    }

    void DropdownWidgetComponent::OnPointerUp() {
    }

    void DropdownWidgetComponent::OnClick() {
        if (!interactable) {
            return;
        }

        if (expanded) {
            // Check if clicked on an option
            auto& input = Engine::GetInstance().GetInputManager();
            glm::vec2 mousePos = input.GetMousePosition();
            
            auto& renderer = Engine::GetInstance().GetSceneRenderer();
            const auto& viewport = renderer.GetRenderingDevice()->GetViewport();
            mousePos.y = viewport.height - mousePos.y;

            int clickedIndex = GetItemAtPosition(mousePos);
            if (clickedIndex >= 0 && clickedIndex < static_cast<int>(options.size())) {
                SetValue(clickedIndex);
                Collapse();
            } else {
                // Clicked outside options, toggle
                Collapse();
            }
        } else {
            Expand();
        }
    }

    int DropdownWidgetComponent::GetItemAtPosition(const glm::vec2& pos) const {
        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        if (!rt || options.empty() || !expanded) {
            return -1;
        }

        float uiScale = Engine::GetInstance().GetSceneRenderer().GetUIScale();
        auto basePos = rt->GetScreenPosition();
        glm::vec2 scaledSize = rt->GetSize() * uiScale;
        float scaledItemHeight = itemHeight * uiScale;

        // Check if in dropdown area
        float dropdownTop = basePos.y;
        float dropdownBottom = basePos.y - scaledItemHeight * options.size();

        if (pos.x < basePos.x || pos.x > basePos.x + scaledSize.x ||
            pos.y > dropdownTop || pos.y < dropdownBottom) {
            return -1;
        }

        // Calculate which item
        float relativeY = dropdownTop - pos.y;
        int index = static_cast<int>(relativeY / scaledItemHeight);
        
        if (index >= 0 && index < static_cast<int>(options.size())) {
            return index;
        }

        return -1;
    }

} // namespace golias
