#include "core/canvas/Text.h"

Text::Text(Font& font, const std::string& text, Transform transform, float font_size) {
    this->font      = font;
    this->text      = text;
    this->transform = transform;
    this->color     = color;
    this->font_size = font_size;
}


void Text::Draw() {
    DrawText(font, text, transform, color, font_size);
}

void Text::SetFont(Font& font) {
    this->font = font;
}

void Text::SetText(const std::string& text) {
    this->text = text;
}

std::string Text::GetText() {
    return text;
}

Font& Text::GetFont() {
    return font;
}

Transform& Text::GetTransform() {
    return transform;
}

void Text::SetTransform(Transform transform) {
    this->transform = transform;
}

void Text::SetColor(Color color) {
    this->color = color;
}

int Text::GetTextWidth() {
    float scale_factor = (font_size > 0.0f) ? (font_size / font.font_size) : 1.0f;
    int text_width     = 0;

    for (char c : text) {
        if (c == '\n') {
            continue;
        }

        if (font.glyphs.find(c) == font.glyphs.end()) {
            continue;
        }

        const Glyph& g = font.glyphs.at(c);

        text_width += (g.advance) * scale_factor;
    }

    return text_width;
}