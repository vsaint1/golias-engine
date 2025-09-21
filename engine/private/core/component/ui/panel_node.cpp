#include "core/component/ui/panel_node.h"

#include "core/ember_core.h"


void Panel::ready() {
    Control::ready();
}

void Panel::draw(Renderer* renderer) {

    if (!_is_flat) {

        renderer->draw_rect_rounded(_panel_rect, rotation, _color, _style.radius_tl, _style.radius_tr, _style.radius_br, _style.radius_bl,
                                    true, _z_index, 2);
    }

    Control::draw(renderer);
}

void Panel::set_panel_rect(const Rect2& rect) {
    _panel_rect = rect;
}

const Rect2& Panel::get_panel_rect() const {
    return _panel_rect;
}

bool Panel::is_flat() const {
    return _is_flat;
}

void Panel::set_flat(bool flat) {
    _is_flat = flat;
}


Panel::~Panel() {
}
