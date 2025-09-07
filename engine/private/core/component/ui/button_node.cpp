#include "core/component/ui/button_node.h"

#include "core/ember_core.h"

void Button::set_text(const std::string& new_text) {
    if (_text != new_text) {
        _text     = new_text;
        _is_dirty = true;
    }
}

void Button::ready() {
    _default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    _pointer_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    Control::ready();
}

void Button::input(const InputManager* input) {
    const glm::vec2 mouse_world = input->screen_to_world(input->get_mouse_position());
    const bool inside           = input->position_in_rect(mouse_world, _btn_rect);

    if (inside && !_is_hovered) {
        SDL_SetCursor(_pointer_cursor);

        if (on_enter) {
            on_enter();
        }
    }

    if (!inside && _is_hovered) {
        SDL_SetCursor(_default_cursor);

        if (on_exit) {
            on_exit();
        }
    }

    _is_hovered = inside;

    if (inside && input->is_mouse_button_pressed(mask) && !_was_pressed) {
        if (on_pressed) {
            on_pressed();
        }
    }

    _was_pressed = inside && input->is_mouse_button_held(mask);
}

void Button::process(double delta_time) {

    Control::process(delta_time);
}

void Button::draw(Renderer* renderer) {
    glm::vec4 color = _style.normal_color;
    if (_was_pressed) {
        color = _style.pressed_color;
    } else if (_is_hovered) {
        color = _style.hover_color;
    }

    if (!_text.empty() && _is_dirty) {
        const glm::vec2 text_size = renderer->calc_text_size(_text, 1.f, _font_alias);

        _btn_rect.width  = SDL_max(min_size.x, text_size.x * 1.2f + _style.padding);
        _btn_rect.height = SDL_max(min_size.y, text_size.y * 2.f + _style.padding);

        text_offset.x = (_btn_rect.width - text_size.x) * 0.5f;
        text_offset.y = (_btn_rect.height - text_size.y) * 0.2f;

        _is_dirty = false;
    }

    if (!is_flat) {
        renderer->draw_rect_rounded(_btn_rect, rotation, color, _style.radius_tl, _style.radius_tr, _style.radius_br, _style.radius_bl,
                                    false, _z_index, 2);
    }

    if (!_text.empty()) {
        renderer->draw_text(_text, _btn_rect.x + text_offset.x, _btn_rect.y + text_offset.y, 0, 1.f, _style.text_color, _font_alias,
                            _z_index + 1, UberShader::none());
    }

    Control::draw(renderer);
}


Button::~Button() {
    SDL_DestroyCursor(_default_cursor);
    SDL_DestroyCursor(_pointer_cursor);
}
