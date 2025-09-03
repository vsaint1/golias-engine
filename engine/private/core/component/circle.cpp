#include "core/component/circle.h"

#include "core/ember_core.h"


void Circle2D::ready() {
    Node2D::ready();
}

void Circle2D::process(double delta_time) {
    Node2D::process(delta_time);
}

void Circle2D::draw(Renderer* renderer) {
    const auto center = get_global_transform().position;

   renderer->draw_circle(center.x, center.y, get_global_transform().rotation, _radius * get_global_transform().scale.x, _color,
                          _is_filled, 32, _z_index);

    Node2D::draw(renderer);
}

void Circle2D::set_filled(bool fill) {
        _is_filled = fill;
}
