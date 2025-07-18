#pragma once
#include "core/component/node.h"


class Polygon2D final : public Node2D {
public:
    explicit Polygon2D(const std::vector<glm::vec2>& points, bool fill = false, Color color = Color::WHITE)
        : _points(points), _color(color), _bFilled(fill) {
    }

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

private:
    std::vector<glm::vec2> _points;
    Color _color  = Color::WHITE;
    bool _bFilled = false;
};
