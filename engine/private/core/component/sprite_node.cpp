#include "core/component/sprite_node.h"

#include "core/ember_core.h"


void Sprite2D::set_region(const Rect2& region, glm::vec2 size) {
    _source     = region;
    _size       = size;
    _use_region = true;
}


void Sprite2D::set_color(const Color& col) {
    _color = col;
}

void Sprite2D::ready() {
    if (_size.x <= 0 || _size.y <= 0) {
        _size = {static_cast<float>(_texture.width), static_cast<float>(_texture.height)};
    }

    const auto position = get_global_transform().position;

    _dest = Rect2{position.x, position.y, _size.x, _size.y};

    _origin = {0.5f, 0.5f};

    Node2D::ready();
}

void Sprite2D::process(double delta_time) {
    Node2D::process(delta_time);
}

void Sprite2D::draw(Renderer* renderer) {
    const auto transform = get_global_transform();

    _dest.x = transform.position.x;
    _dest.y = transform.position.y;

    SDL_assert(_texture.id > 0);

    if (_use_region) {
        renderer->draw_texture(_texture,_dest,transform.rotation,glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), _source, _z_index);
    } else {
        renderer->draw_texture(_texture, _dest, transform.rotation, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {}, 5);
    }

    Node2D::draw(renderer);
}

void Sprite2D::input(const InputManager* input) {
}
