#include "core/canvas/button.h"


Button::Button(Font& font, const std::string& label, ember::Rectangle rect, float font_size) : rect(rect) {

    Text button_text;
    button_text.SetFont(font);
    button_text.SetText(label);
    button_text.SetColor({255, 255, 255, 255});
    button_text.SetFontSize(font_size);


    button_text.SetTransform({
        glm::vec3(rect.x + padding, rect.y + (rect.height / 2) + padding, 1.f),
        glm::vec3(0.f),
        glm::vec3(1.f),
    });

    text = button_text;

    CalcSize();


}


void Button::Draw() {


    text.Draw();

    DrawRectFilled(rect, color);

    if (IsHovered()) {
        DrawRect(rect, {255, 255, 255, 100}, 4.f);
    }
}
