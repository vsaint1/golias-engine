#include "core/component/ui/button_node.h"

#include "core/ember_core.h"


void Button::ready() {
    _default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    _pointer_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    Control::ready();
}

void Button::input(const InputManager* input) {
    glm::vec2 mouse_world = input->screen_to_world(input->get_mouse_position());
    const bool inside     = input->position_in_rect(mouse_world, _btn_rect);

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

    if (inside && input->is_mouse_button_pressed(MouseButton::LEFT) && !_was_pressed) {
        if (on_pressed) {
            on_pressed();
        }
    }

    _was_pressed = inside && input->is_mouse_button_held(MouseButton::LEFT);
}

void Button::process(double delta_time) {

    Control::process(delta_time);
}

void Button::draw(Renderer* renderer) {
    glm::vec4 color = style.normal_color;
    if (_was_pressed) {
        color = style.pressed_color;
    } else if (_is_hovered) {
        color = style.hover_color;
    }


    renderer->draw_rect_rounded(_btn_rect, rotation, color, style.radius_tl, style.radius_tr, style.radius_br, style.radius_bl, true,
                                _z_index);

    if (!text.empty()) {
        const glm::vec2 text_size = renderer->calc_text_size(text, 1.f, "Default");
        if (_text_size == glm::vec2(0.0f) || _text_size != text_size) {
            _text_size = text_size;

            _btn_rect.width  = min_size.x + text_size.x * 1.2f + style.padding;
            _btn_rect.height = min_size.y + text_size.y * 2.f + style.padding;

            text_offset.x = style.padding;
            text_offset.y = style.padding;
        }

        renderer->draw_text(text, _btn_rect.x + text_offset.x, _btn_rect.y + text_offset.y, 0, 1.f, style.text_color, "Default", _z_index,
                            UberShader::none());
    }

    Control::draw(renderer);
}


Button::~Button() {
    SDL_DestroyCursor(_default_cursor);
    SDL_DestroyCursor(_pointer_cursor);
}
