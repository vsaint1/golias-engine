#pragma once
#include "scene/ui/ui_base.h"
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

namespace golias {
    class Font;
    
    class TextWidgetComponent : public WidgetComponent {
        COMPONENT_DERIVED(TextWidgetComponent, WidgetComponent)
    public:
        const std::string& GetText() const;
        void SetText(const std::string& txt);
        
        glm::vec4 GetTextColor() const;
        void SetTextColor(const glm::vec4& color);
        
        void SetFont(const std::shared_ptr<Font>& pFont);
        const std::shared_ptr<Font>& GetFont() const;
        void SetFont(const std::string_view pFilePath, int size);
        
        void SetOutlineEnabled(bool enabled);
        bool IsOutlineEnabled() const;
        void SetOutlineColor(const glm::vec4& color);
        glm::vec4 GetOutlineColor() const;
        void SetOutlineThickness(float thickness);
        float GetOutlineThickness() const;
        
        void SetShadowEnabled(bool enabled);
        bool IsShadowEnabled() const;
        void SetShadowColor(const glm::vec4& color);
        glm::vec4 GetShadowColor() const;
        void SetShadowOffset(const glm::vec2& offset);
        glm::vec2 GetShadowOffset() const;
        
        void Start() override;
        void LoadProperties(const nlohmann::json& json) override;
        void Draw(CanvasComponent* pCanvas) override;
        glm::vec2 GetPivotPos() const;
        
    protected:
        std::string text;
        glm::vec4 textColor = glm::vec4(1.0f);
        std::shared_ptr<Font> font = nullptr;
        
        bool outlineEnabled = false;
        glm::vec4 outlineColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float outlineThickness = 1.0f;
        
        bool shadowEnabled = false;
        glm::vec4 shadowColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
        glm::vec2 shadowOffset = glm::vec2(2.0f, -2.0f);
        
    };
} // namespace golias