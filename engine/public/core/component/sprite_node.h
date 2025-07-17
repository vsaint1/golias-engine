#pragma once
#include "node.h"

class SpriteNode final : public Node2D {
public:
    Texture _texture;
    glm::vec2 _size = {};
    Color _color    = {255, 255, 255, 255};

    explicit SpriteNode(const Texture& tex, const Color col = Color(255, 255, 255, 255), const glm::vec2 size = {0, 0})
        : _texture(tex), _size(size), _color(col) {
    }

    void set_color(const Color& col);

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

    void event(const InputManager* input) override;
};
