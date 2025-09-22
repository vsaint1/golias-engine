#pragma once

#include  "core/component/ui/panel_node.h"


class LineEdit final : public Panel {
public:
    LineEdit(const glm::vec2& pos, const glm::vec2& size, const std::string& placeholder_text = "",const std::string& font_alias = "Default");

    void ready() override;
    void draw(Renderer* renderer) override;
    void input(const InputManager* input) override;
    void process(double dt) override;

    const std::string& get_text() const;
    void set_text(const std::string& text);
    void reset();

    void focus();
    void unfocus();

    bool is_focused() const;
    bool has_text_input_active() const;

    ~LineEdit() override;

    std::function<void()> on_enter;
    std::function<void()> on_exit;
    std::function<void(const std::string&)> on_text_changed;

private:
    float padding          = 4.0f;
    float char_width       = 16.0f;
    float line_height      = 16.0f;
    SDL_Rect _text_input_rect;

    std::string _font_alias;

    std::string _placeholder_text = "";

    std::string _text;
    int _cursor_pos;
    bool _show_cursor;
    double _cursor_blink_timer;
    bool _is_focused;
    bool _is_hovered;

    std::string _composition_text;
    int _composition_cursor;
    int _composition_selection_len;

    SDL_Cursor* _pointer_cursor;
    SDL_Cursor* _default_cursor;

    void handle_text_input(const std::string& text);
    void handle_text_editing(const SDL_TextEditingEvent& event);
    void handle_key_input(const SDL_KeyboardEvent& event);

    void clear_composition();
    void update_text_input_rect();
};