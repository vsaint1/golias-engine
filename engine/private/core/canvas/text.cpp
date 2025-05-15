#include "core/canvas/text.h"

Text::Text(const Font& font, const std::string& text, const Transform& transform, float font_size, const Color& color_) {
    this->font      = font;
    this->text      = text;
    this->transform = transform;
    this->color     = color_;
    this->font_size = font_size;
}


void Text::Draw() const{
    GEngine->GetRenderer()->DrawText(font, text, transform, color, font_size,{});
}

void Text::SetFont(const Font& font) {
    this->font = font;
}

void Text::SetText(const std::string& text) {
    this->text = text;
}

const std::string Text::GetText()  {
    return text;
}

const Font& Text::GetFont() {
    return font;
}

const Transform& Text::GetTransform() {
    return transform;
}

const float Text::GetFontSize() {
    return font_size;
}

void Text::SetTransform(const Transform& transform_) {
    this->transform = transform_;
}

void Text::SetColor(const Color& color_) {
    this->color = color_;
}
void Text::SetFontSize(float size) {
    this->font_size = size;
}
