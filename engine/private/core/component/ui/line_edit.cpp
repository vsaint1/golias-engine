#include "core/component/ui/line_edit.h"

#include "core/ember_core.h"

LineEdit::LineEdit(const glm::vec2& pos, const glm::vec2& size, const std::string& placeholder_text, const std::string& font_alias)
    : _font_alias(font_alias), _placeholder_text(placeholder_text), _text(""), _cursor_pos(0), _show_cursor(true), _cursor_blink_timer(0.0),
      _is_focused(false), _is_hovered(false), _composition_text(""), _composition_cursor(0), _composition_selection_len(0) {
    _panel_rect      = {pos.x, pos.y, size.x, size.y};
    _text_input_rect = {(int) pos.x + 4, (int) pos.y + 4, (int) size.x - 8, (int) size.y - 8};
}

void LineEdit::ready() {
    Panel::ready();
}

void LineEdit::draw(Renderer* renderer) {
    Panel::draw(renderer);

    _color = _style.normal_color;
    if (_is_hovered) {
        _color = _style.hover_color;
    } else if (_is_disabled) {
        _color = _style.disabled_color;
    }

    int max_chars_per_line = (int) ((_panel_rect.width - 2 * padding) / char_width);
    int max_lines          = (int) ((_panel_rect.height - 2 * padding) / line_height);



    std::string wrapped_text;
    int line_count = 0;
    for (size_t i = 0; i < _text.size() && line_count < max_lines; i += max_chars_per_line) {
        size_t len = SDL_min((size_t) max_chars_per_line, _text.size() - i);

        std::string line = _text.substr(i, len);
        line_count++;


        if (line_count == max_lines && i + len < _text.size()) {
            if (line.size() > 3) {
                line = line.substr(0, line.size() - 3) + "...";
            } else {
                line = "...";
            }
        }

        wrapped_text += line;

        if (i + len < _text.size() && line_count < max_lines) {
            wrapped_text += "\n";
        }
    }

    if (!_is_focused && _text.empty() && !_placeholder_text.empty()) {
        renderer->draw_text(_placeholder_text, _panel_rect.x + padding, _panel_rect.y + padding, 0, 1.0f, _style.text_placeholder_color,
                            _font_alias, _z_index + 1);
    }


    renderer->draw_text(wrapped_text, _panel_rect.x + padding, _panel_rect.y + padding, 0, 1.0f, _style.text_color, _font_alias, _z_index + 1);


    if (_show_cursor && _is_focused) {
        int visual_cursor_pos = _cursor_pos + _composition_cursor;
        int cursor_line       = visual_cursor_pos / max_chars_per_line;
        int cursor_col        = visual_cursor_pos % max_chars_per_line;
        float cursor_x        = _panel_rect.x + padding + cursor_col * char_width;
        float cursor_y        = _panel_rect.y + padding + cursor_line * line_height;

        if (cursor_line < max_lines) {
            renderer->draw_rect({cursor_x, cursor_y, 2, line_height}, 0.0f, _style.text_color, true, _z_index);
        }
    }
}

void LineEdit::input(const InputManager* input) {
    glm::vec2 mouse = input->screen_to_world(input->get_mouse_position());
    bool inside     = input->position_in_rect(mouse, _panel_rect);

    if (inside && !_is_hovered) {
        if (on_enter) {
            on_enter();
        }
    }
    if (!inside && _is_hovered) {
        if (on_exit) {
            on_exit();
        }
    }
    _is_hovered = inside;

    if (inside && input->is_mouse_button_pressed(MouseButton::LEFT)) {
        focus();
        int max_chars_per_line = (int) ((_panel_rect.width - 2 * padding) / 8.0f);
        int row                = (int) ((mouse.y - _panel_rect.y - padding) / 16);
        int col                = (int) ((mouse.x - _panel_rect.x - padding) / 8.0f);
        int clicked_pos        = row * max_chars_per_line + col;
        _cursor_pos            = std::clamp(clicked_pos, 0, (int) _text.size());
    }

    if (!inside && input->is_mouse_button_pressed(MouseButton::LEFT)) {
        unfocus();
    }

    if (_is_focused) {
        const SDL_Event& event = input->get_last_event();
        switch (event.type) {
        case SDL_EVENT_TEXT_INPUT:
            handle_text_input(input->get_last_text_input());
            break;
        case SDL_EVENT_TEXT_EDITING:
            handle_text_editing(event.edit);
            break;
        case SDL_EVENT_KEY_DOWN:
            handle_key_input(event.key);
            break;
        default:
            break;
        }
    }
}

void LineEdit::process(double dt) {
    _cursor_blink_timer += dt;
    if (_cursor_blink_timer > 0.5) {
        _show_cursor        = !_show_cursor;
        _cursor_blink_timer = 0.0;
    }
}

const std::string& LineEdit::get_text() const {
    return _text;
}

void LineEdit::set_text(const std::string& text) {
    _text       = text;
    _cursor_pos = (int) text.size();
    clear_composition();
}

void LineEdit::reset() {
    _text       = "";
    _cursor_pos = 0;
    clear_composition();
    unfocus();
}

void LineEdit::focus() {
    if (!_is_focused) {
        _is_focused = true;
        SDL_StartTextInput(GEngine->Window.handle);
        SDL_SetTextInputArea(GEngine->Window.handle, &_text_input_rect, 0);
    }
}

void LineEdit::unfocus() {
    if (_is_focused) {
        _is_focused = false;
        SDL_StopTextInput(GEngine->Window.handle);
        clear_composition();
    }
}

bool LineEdit::is_focused() const {
    return _is_focused;
}
bool LineEdit::has_text_input_active() const {
    return SDL_TextInputActive(GEngine->Window.handle);
}

LineEdit::~LineEdit() {
    if (_is_focused) {
        SDL_StopTextInput(GEngine->Window.handle);
    }

}

// --- Private methods ---
void LineEdit::handle_text_input(const std::string& text) {
    if (!text.empty()) {
        _text.insert(_cursor_pos, text);
        _cursor_pos += static_cast<int>(text.length());
        clear_composition();
        if (on_text_changed) {
            on_text_changed(_text);
        }
    }
}

void LineEdit::handle_text_editing(const SDL_TextEditingEvent& event) {
    _composition_cursor        = event.start;
    _composition_selection_len = event.length;
    update_text_input_rect();
}

void LineEdit::handle_key_input(const SDL_KeyboardEvent& event) {
    if (event.repeat) {
        return;
    }

    switch (event.scancode) {
    case SDL_SCANCODE_BACKSPACE:
        if (_cursor_pos > 0) {
            _text.erase(_cursor_pos - 1, 1);
            _cursor_pos--;
            clear_composition();
            if (on_text_changed) {
                on_text_changed(_text);
            }
        }
        break;
    case SDL_SCANCODE_DELETE:
        if (_cursor_pos < (int) _text.size()) {
            _text.erase(_cursor_pos, 1);
            clear_composition();
            if (on_text_changed) {
                on_text_changed(_text);
            }
        }
        break;
    case SDL_SCANCODE_LEFT:
        if (_cursor_pos > 0) {
            _cursor_pos--;
            clear_composition();
            update_text_input_rect();
        }
        break;
    case SDL_SCANCODE_RIGHT:
        if (_cursor_pos < (int) _text.size()) {
            _cursor_pos++;
            clear_composition();
            update_text_input_rect();
        }
        break;
    case SDL_SCANCODE_HOME:
        _cursor_pos = 0;
        clear_composition();
        update_text_input_rect();
        break;
    case SDL_SCANCODE_END:
        _cursor_pos = (int) _text.size();
        clear_composition();
        update_text_input_rect();
        break;
    case SDL_SCANCODE_RETURN:
        if (on_enter) {
            on_enter();
        }
        break;
    case SDL_SCANCODE_ESCAPE:
        unfocus();
        break;
    default:
        break;
    }
}

void LineEdit::clear_composition() {
    _composition_text.clear();
    _composition_cursor        = 0;
    _composition_selection_len = 0;
}

void LineEdit::update_text_input_rect() {
    if (_is_focused) {

        int max_chars_per_line = (int) ((_panel_rect.width - 2 * padding) / char_width);

        int cursor_line = _cursor_pos / max_chars_per_line;
        int cursor_col  = _cursor_pos % max_chars_per_line;

        _text_input_rect.x = (int) (_panel_rect.x + padding + cursor_col * char_width);
        _text_input_rect.y = (int) (_panel_rect.y + padding + cursor_line * line_height);
        _text_input_rect.w = (int) (char_width * 10);
        _text_input_rect.h = (int) line_height;

        SDL_SetTextInputArea(GEngine->Window.handle, &_text_input_rect, 0);
    }
}
