#pragma once
#include "scene/ui/ui_base.h"
#include <functional>
#include <vector>
#include <string>
#include <glm/vec4.hpp>

namespace golias {

    class Font;

    using DropdownCallback = std::function<void(int, const std::string&)>;

    /// @brief Dropdown component for selection from a list
    class DropdownWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(DropdownWidgetComponent, WidgetComponent)
    public:
        void AddOption(const std::string& option);
        void ClearOptions();
        void SetOptions(const std::vector<std::string>& options);
        const std::vector<std::string>& GetOptions() const;

        void SetValue(int index);
        int GetValue() const;

        const std::string& GetSelectedText() const;

        void SetInteractable(bool interactable);
        bool IsInteractable() const;

        void SetBackgroundColor(const glm::vec4& color);
        glm::vec4 GetBackgroundColor() const;

        void SetHighlightColor(const glm::vec4& color);
        glm::vec4 GetHighlightColor() const;

        void SetTextColor(const glm::vec4& color);
        glm::vec4 GetTextColor() const;

        void SetDropdownBackgroundColor(const glm::vec4& color);
        glm::vec4 GetDropdownBackgroundColor() const;

        void SetItemHeight(float height);
        float GetItemHeight() const;

        void SetFont(const std::shared_ptr<Font>& pFont);
        const std::shared_ptr<Font>& GetFont() const;
        void SetFont(const std::string_view path, int size);
        void SetFontSize(int size);
        int GetFontSize() const;

        bool IsExpanded() const;
        void Expand();
        void Collapse();

        void Start() override;
        void Update(float deltaTime) override;
        void Draw(CanvasComponent* pCanvas) override;
        void LoadProperties(const nlohmann::json& json) override;

        bool HitTest(const glm::vec2& point) const override;
        void OnPointerDown() override;
        void OnPointerUp() override;
        void OnClick() override;

        /// @brief Called when selection changes
        DropdownCallback OnValueChanged;

    private:
        int GetItemAtPosition(const glm::vec2& pos) const;
        
        std::vector<std::string> options;
        int selectedIndex = 0;
        
        bool interactable = true;
        bool expanded = false;
        int hoveredIndex = -1;
        
        glm::vec4 backgroundColor = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
        glm::vec4 highlightColor = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
        glm::vec4 textColor = glm::vec4(1.0f);
        glm::vec4 dropdownBackgroundColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        
        float itemHeight = 30.0f;
        
        std::shared_ptr<Font> font = nullptr;
        int fontSize = 14;
        
        static const std::string emptyOption;
    };

} // namespace golias
