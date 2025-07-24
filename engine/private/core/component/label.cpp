#include "core/component/label.h"

#include "core/ember_core.h"


Label::Label(const std::string& font_path, const std::string& text, const float ft_size, const Color& color)
    : _text(text), _color(color), _font_size(ft_size), _path(font_path) {

    _font = GEngine->get_renderer()->load_font(font_path, ft_size);
}

void Label::ready() {
}

void Label::process(double delta_time) {
    Node2D::process(delta_time);
}

void Label::draw(Renderer* renderer) {
    renderer->draw_text(_font, _text, get_global_transform(), _color, _font_size, _effect, 0);
    Node2D::draw(renderer);
}

void Label::set_text(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    const int size = SDL_vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (size <= 0) {
        return;
    }

    std::string buffer(size, '\0');

    va_start(args, fmt);
    SDL_vsnprintf(&buffer[0], size + 1, fmt, args);
    va_end(args);

    _text = std::move(buffer);
}


void Label::set_font_size(float size) {
    _font_size = size;
}

void Label::set_text_color(const Color& color) {
    _color = color;
}

void Label::set_outline(bool enabled, float thickness, const Color& color) {
    _effect.Outline.enabled = enabled ? 1 : 0;
    _effect.Outline.thickness = thickness;
    _effect.Outline.color = color.normalize_color();
}

void Label::set_shadow(bool enabled, glm::vec2 offset, const Color& color) {
    _effect.Shadow.enabled = enabled ? 1 : 0;
    _effect.Shadow.pixel_offset = offset;
    _effect.Shadow.color = color.normalize_color();
}
