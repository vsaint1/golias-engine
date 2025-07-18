#pragma once
#include "core/component/node.h"


class Circle2D final : public Node2D {

public:


    explicit Circle2D(float radius = 10.0f, bool fill = false, Color color = Color::WHITE)
        : _radius(radius), _color(color), _is_filled(fill) {
    }

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;
private:
    float _radius = 10.0f;
    int _segments = 32;
    Color _color  = Color::WHITE;
    bool _is_filled = true;
};
