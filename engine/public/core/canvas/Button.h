#pragma once
#include "Text.h"
#include "core/engine.h"

class Button {
public:
    Button() = default;

    Button(Font& font, const std::string& label, ember::Rectangle rect, float font_size = 0.0f);

    void Draw();

    void SetFontSize(float size) {
        text.SetFontSize(size);
    }

    void SetRect(ember::Rectangle rect) {
        this->rect = rect;
    }

    void SetFont(Font& font) {
        text.SetFont(font);
    }

    void SetText(const std::string& newText) {
        this->text.SetText(newText);
    }

    void SetPadding(float padding) {
        this->padding = padding;
    }


    void SetTextColor(const Color& color) {
        text.SetColor(color);
    }

    void SetBackgroundColor(const Color& color) {
        this->color = color;
    }

    bool IsHovered() const {
        return core.Input->IsPositionInRect(core.Input->GetMousePosition(), rect);
    }

    bool WasClicked() const {
        return IsHovered() && core.Input->IsMouseButtonPressed(SDL_BUTTON_LEFT);
    }

private:
    void CalcSize() {
        if (rect.width > text.GetTextWidth()) {
            return;
        }

        rect.width = text.GetTextWidth() + padding;
    }


    Text text;
    ember::Rectangle rect;
    Color color   = {0, 0, 0, 100};
    float padding = 10.f;
};
