#pragma once
#include "node.h"


class Label final : public Node2D {

public:
    explicit Label(Font font, const std::string& text, const float ft_size, const Color& color = Color::WHITE)
        : _font(std::move(font)), _text(text), _color(color), _font_size(ft_size) {
    }

    Label(const std::string& font_path, const std::string& text, const float ft_size, const Color& color = Color::WHITE);

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

    void set_text(const char* fmt, ...);

    void set_font_size(float size);

    void set_text_color(const Color& color);

    void set_outline(bool enabled, float thickness, const Color& color = Color::BLACK);

    void set_shadow(bool enabled, glm::vec2 offset = glm::vec2(1.f), const Color& color = Color::BLACK);

private:
    Font _font;
    std::string _text    = "";
    Color _color         = Color::WHITE;
    ShaderEffect _effect = {}; // for now just drawing text without effects
    int _font_size       = 16;
    std::string _path    = "";
};
