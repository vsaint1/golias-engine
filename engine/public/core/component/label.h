#pragma once
#include "node.h"


class Label final : public Node2D {

public:
    explicit Label(const std::string& font_alias, const std::string& text, const int ft_size = 0, const Color& color = Color::WHITE)
        : _font_alias(font_alias), _text(text), _color(color), _font_size(ft_size) {
    }

    Label(const std::string& font_path, const std::string& font_alias, const std::string& text, int ft_size = 16, const Color& color = Color::WHITE);

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

    void set_text(const char* fmt, ...);

    void set_font_size(float size);

    void set_text_color(const Color& color);

    void set_outline(bool enabled, float thickness = 1.f, const Color& color = Color::BLACK);

    void set_shadow(bool enabled, glm::vec2 offset = glm::vec2(-1.f), const Color& color = Color::BLACK);


private:
    std::string _font_alias = "default";
    std::string _text    = "";
    Color _color         = Color::WHITE;
    UberShader _effect = UberShader::none();
    int _font_size       = 0;
    float _kerning       = 0.0f; // spacing between characters
    std::string _path    = "";
};
