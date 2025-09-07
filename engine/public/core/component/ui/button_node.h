#pragma once
#include "control_node.h"

class Button : public Control {
public:
    glm::vec2 text_offset = {0.0f, 0.0f};
    glm::vec2 min_size    = {0, 0};
    bool is_flat = false;
    MouseButton mask = MouseButton::LEFT;

    Button(const char* txt , const glm::vec2& pos, const std::string& font_alias = "Default",const glm::vec2& size = {0,0}) {
        _text = txt;
        _font_alias = font_alias;
        _btn_rect.x      = pos.x;
        _btn_rect.y      = pos.y;
        _btn_rect.width  = min_size.x;
        _btn_rect.height = min_size.y;
         min_size         = size;

        _z_index = 9999;

        _is_dirty            = true;

    }


    std::function<void()> on_enter;
    std::function<void()> on_exit;
    std::function<void()> on_pressed;

    // TODO: this need to be fixed, changing the button affects other ???
    void set_text(const std::string& new_text);

    void ready() override;
    void input(const InputManager* input) override;
    void process(double delta_time) override;
    void draw(Renderer* renderer) override;

    ~Button() override;

private:
    std::string _text;
    std::string _font_alias = "Default";

    Rect2 _btn_rect;

    bool _pressed_inside = false;
    bool _was_pressed = false;
    bool _is_hovered  = false;

    SDL_Cursor* _default_cursor = nullptr;
    SDL_Cursor* _pointer_cursor = nullptr;
};
