#pragma once
#include "node.h"

class Sprite2D final : public Node2D {

public:
    explicit Sprite2D(const Texture& tex, const Color col = Color(255, 255, 255, 255), const glm::vec2 size = {0, 0})
        : _texture(tex), _size(size), _color(col) {
    }


    void set_region(const Rect2& region, glm::vec2 size);

    void set_color(const Color& col);

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

    void input(const InputManager* input) override;

private:
    Texture _texture;
    glm::vec2 _size          = glm::vec2(0.f);
    Color _color             = {255, 255, 255, 255};
    Rect2 _source = {};
    Rect2 _dest   = {};
    glm::vec2 _origin        = {};
    bool _use_region         = false;
};
