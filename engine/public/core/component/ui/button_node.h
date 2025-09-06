#pragma once
#include "control_node.h"

class Button : public Control {
public:
    std::string text;
    glm::vec2 text_offset = {0.0f, 0.0f};
    glm::vec2 min_size    = {0, 0};

    Button(const char* txt, const glm::vec2& pos, const glm::vec2& size = {0,0}) {
        text = txt;
        _btn_rect.x      = pos.x;
        _btn_rect.y      = pos.y;
         min_size         = size;
        _btn_rect.width  = min_size.x;
        _btn_rect.height = min_size.y;

        _z_index = 9999;
    }


    std::function<void()> on_enter;
    std::function<void()> on_exit;
    std::function<void()> on_pressed;

    void ready() override;
    void input(const InputManager* input) override;
    void process(double delta_time) override;
    void draw(Renderer* renderer) override;

    ~Button() override;

private:
    glm::vec2 _text_size = {0.0f, 0.0f};
    Rect2 _btn_rect;

    bool _was_pressed = false;
    bool _is_hovered  = false;

    SDL_Cursor* _default_cursor = nullptr;
    SDL_Cursor* _pointer_cursor = nullptr;
};
