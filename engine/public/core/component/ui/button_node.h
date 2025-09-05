#pragma once
#include "control_node.h"

class Button : public Control {
public:
    std::string text = "Button";

    Button(const std::string& txt, const glm::vec2& pos, const glm::vec2& size) : text(txt) {
        _btn_rect.x      = pos.x;
        _btn_rect.y      = pos.y;
        _btn_rect.width  = size.x;
        _btn_rect.height = size.y;
        _z_index = 9999;
    }


    std::function<void()> on_hover_enter;
    std::function<void()> on_hover_exit;
    std::function<void()> on_pressed;

    void ready() override;
    void input(const InputManager* input) override;
    void process(double delta_time) override;
    void draw(Renderer* renderer) override;

    ~Button() override;

private:
    Rect2 _btn_rect;

    bool _was_pressed = false;
    bool _is_hovered    = false;

    SDL_Cursor* _default_cursor = nullptr;
    SDL_Cursor* _pointer_cursor = nullptr;


};
