#pragma once
#include "text.h"
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
        InputManager* input_manager = GEngine->GetInputManager();

        return input_manager->IsPositionInRect(input_manager->GetMousePosition(), rect);
    }

    bool WasClicked() const {
        return IsHovered() && GEngine->GetInputManager()->IsMouseButtonPressed(SDL_BUTTON_LEFT);
    }

private:
    void CalcSize() {
        auto[text_width, text_height] = CalcTextSize(text.GetText(), text.GetFont(), text.GetFontSize());

        if (rect.width > text_width) {
            return;
        }

        rect.width = text_width + padding;
    }


    Text text;
    ember::Rectangle rect;
    Color color   = {0, 0, 0, 100};
    float padding = 10.f;
};
