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

        void LoadProperties(const nlohmann::json& json) override;

        void Draw(CanvasComponent* pCanvas) override;

        glm::vec2 GetPivotPos() const;
        
    protected:
        std::string text;
        glm::vec4 textColor = glm::vec4(1.0f);
        std::shared_ptr<Font> font;
    };
} // namespace golias
