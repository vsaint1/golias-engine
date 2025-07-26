#include "core/component/label.h"

#include "core/ember_core.h"


void Label::ready() {
    Node2D::ready();
}

void Label::process(double delta_time) {
    Node2D::process(delta_time);
}

void Label::draw(Renderer* renderer) {
    const auto& transform = get_global_transform();
    GEngine->get_renderer()->draw_text(_text, transform.position.x,transform.position.y, transform.rotation, transform.scale.x, _color.normalize_color(), _font_alias, _z_index);

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
