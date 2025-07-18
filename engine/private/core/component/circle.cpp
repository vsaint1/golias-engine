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

    if (_is_filled) {
        renderer->draw_circle_filled(glm::vec3(center, 1.f), _radius, _color, _segments);
    } else {
        renderer->draw_circle(glm::vec3(center, 1.f), _radius, _color, _segments);
    }

    Node2D::draw(renderer);
}
