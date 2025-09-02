#pragma once
#include "node.h"

/*!
 *  @brief 2D Sprite Node
 *
 *  @details  A 2D sprite node that can be added to the scene graph.
 *  - Supports texture, color tinting, and region selection.
 *  - Can be flipped horizontally and vertically.
 *
 *  @version 1.1.0
 */
class Sprite2D final : public Node2D {

public:

    explicit Sprite2D(const std::weak_ptr<Texture>& tex, const Color col = Color(255, 255, 255, 255), const glm::vec2 size = {0, 0})
      : _texture(tex), _size(size), _color(col) {
    }

    [[nodiscard]] Rect2 get_region() const;

    void set_region(const Rect2& region, glm::vec2 size);

    void set_color(const Color& col);

    void ready() override;

    void process(double delta_time) override;

    void draw(Renderer* renderer) override;

    void input(const InputManager* input) override;

    void set_flip_horizontal(bool flip) { _flip_h = flip; }
    void set_flip_vertical(bool flip) { _flip_v = flip; }

    bool is_flipped_horizontal() const { return _flip_h; }
    bool is_flipped_vertical() const { return _flip_v; }

    glm::vec2 get_size() const { return _size; }

    void set_texture(const std::weak_ptr<Texture>& tex) { _texture = tex; }
private:
    std::weak_ptr<Texture> _texture;
    glm::vec2 _size          = glm::vec2(0.f);
    Color _color             = {255, 255, 255, 255};
    Rect2 _source = {};
    Rect2 _dest   = {};
    glm::vec2 _origin        = {};
    bool _use_region         = false;

    bool _flip_h = false;
    bool _flip_v = false;
};
