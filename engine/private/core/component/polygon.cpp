#include "core/component/polygon.h"

#include "core/ember_core.h"


void Polygon2D::ready() {
    Node2D::ready();
}

void Polygon2D::process(double delta_time) {
    Node2D::process(delta_time);
}

void Polygon2D::draw(Renderer* renderer) {

    const auto t = get_global_transform();
    std::vector<glm::vec2> transformed_points;

    transformed_points.reserve(_points.size());

    for (const auto& p : _points) {
        transformed_points.emplace_back(t.transform_point(p));
    }

    // renderer->draw_polygon(transformed_points, _color, _is_filled);
    Node2D::draw(renderer);
}

void Polygon2D::set_filled(bool fill) {
    _is_filled  = fill;
}

