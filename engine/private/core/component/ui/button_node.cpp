#include "core/component/ui/button_node.h"

#include "core/ember_core.h"

Button::Button(const char* txt, const glm::vec2& pos, const std::string& font_alias, const glm::vec2& size) {
    _text = txt;
    _font_alias = font_alias;
    _panel_rect.x      = pos.x;
    _panel_rect.y      = pos.y;
    _panel_rect.width  = min_size.x;
    _panel_rect.height = min_size.y;
    min_size         = size;
    _is_dirty            = true;
}

void Button::set_disabled(bool disabled) {
    _is_disabled = disabled;
}

void Button::set_text(const std::string& new_text) {
    if (_text != new_text) {
        _text     = new_text;
        _is_dirty = true;
    }
}

void Button::ready() {

    Control::ready();
}

void Button::input(const InputManager* input) {
    if (_is_disabled) {
        return;
    }

    const glm::vec2 mouse_world = input->screen_to_world(input->get_mouse_position());
    const bool inside_panel     = input->position_in_rect(mouse_world, _panel_rect);

    if (inside_panel && !_is_hovered) {
        if (on_enter) {
            on_enter();
        }
    }

    if (!inside_panel && _is_hovered) {
        if (on_exit) {
            on_exit();
        }
    }

    _is_hovered = inside_panel;

    if (inside_panel && input->is_action_pressed("ui_accept")) {
        _was_pressed = true;
    }

    if (_was_pressed && !input->is_action_held("ui_accept")) {
        if (inside_panel && on_pressed) {
            on_pressed();
        }
        _was_pressed = false;
    }
}

void Button::process(double delta_time) {

    Control::process(delta_time);
}

void Button::draw(Renderer* renderer) {
    _color = _style.normal_color;
    if (_was_pressed) {
        _color = _style.pressed_color;
    } else if (_is_hovered) {
        _color = _style.hover_color;
    } else if (_is_disabled) {
        _color = _style.disabled_color;
    }

    if (!_text.empty() && _is_dirty) {
        const glm::vec2 text_size = renderer->calc_text_size(_text, 1.f, _font_alias);

        _panel_rect.width  = SDL_max(min_size.x, text_size.x + _style.padding);
        _panel_rect.height = SDL_max(min_size.y, text_size.y * 2.f + _style.padding);

        text_offset.x = (_panel_rect.width - text_size.x) * 0.5f;
        text_offset.y = (_panel_rect.height - text_size.y) * 0.2f;

        _is_dirty = false;
    }


    if (!_text.empty()) {
        renderer->draw_text(_text, _panel_rect.x + text_offset.x, _panel_rect.y + text_offset.y, 0, 1.f, _style.text_color, _font_alias,
                            _z_index + 1, UberShader::none());
    }

    Panel::draw(renderer);
}


Button::~Button() {
}
