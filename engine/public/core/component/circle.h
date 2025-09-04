#pragma once
#include "core/component/node.h"


class Circle2D final : public Node2D {

public:


    explicit Circle2D(float radius = 10.0f, bool fill = false, const Color& col = Color::WHITE)
        : _radius(radius), _color(col.normalize_color()), _is_filled(fill) {
    }

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

    void set_filled(bool fill);

private:
    float _radius = 10.0f;
    int _segments = 32;
    glm::vec4 _color  = glm::vec4(1.f);

    bool _is_filled = true;
};
