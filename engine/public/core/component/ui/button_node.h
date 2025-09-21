#pragma once
#include "panel_node.h"

class Button final : public Panel {
public:
    glm::vec2 text_offset = {0.0f, 0.0f};
    glm::vec2 min_size    = {0, 0};
    MouseButton mask = MouseButton::LEFT;

    Button(const char* txt , const glm::vec2& pos, const std::string& font_alias = "Default",const glm::vec2& size = {0,0});

    std::function<void()> on_enter;
    std::function<void()> on_exit;
    std::function<void()> on_pressed;

    void set_disabled(bool disabled);

    void set_text(const std::string& new_text);

    void ready() override;
    void input(const InputManager* input) override;
    void process(double delta_time) override;
    void draw(Renderer* renderer) override;

    ~Button() override;

private:
    std::string _text;
    std::string _font_alias = "Default";

    bool _pressed_inside = false;
    bool _was_pressed = false;
    bool _is_hovered  = false;

    SDL_Cursor* _default_cursor = nullptr;
    SDL_Cursor* _pointer_cursor = nullptr;
};
