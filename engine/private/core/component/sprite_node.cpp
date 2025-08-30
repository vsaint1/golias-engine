#include "core/component/sprite_node.h"

#include "core/ember_core.h"


Rect2 Sprite2D::get_region() const {
    return _source;
}

void Sprite2D::set_region(const Rect2& region, glm::vec2 size) {
    _source     = region;
    _size       = size;
    _use_region = true;
}


void Sprite2D::set_color(const Color& col) {
    _color = col;
}

void Sprite2D::ready() {

    if (_is_ready) {
        return;
    }

    _is_ready = true;

    const auto tex = _texture.lock();

    if (!tex) {
        LOG_ERROR("Failed to access texture in Sprite2D");
    }

    if (_size.x <= 0 || _size.y <= 0) {
        _size = {static_cast<float>(tex->width), static_cast<float>(tex->height)};
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

    if (const auto tex = _texture.lock()) {
        SDL_assert(tex->id > 0);

        if (_use_region) {
            renderer->draw_texture(tex.get(), _dest, transform.rotation, glm::vec4(1.0f), _source, _z_index,_flip_h, _flip_v);
        } else {
            renderer->draw_texture(tex.get(), _dest, transform.rotation, glm::vec4(1.0f), {}, _z_index,_flip_h, _flip_v);
        }

    } else {
        LOG_ERROR("Failed to access texture in Sprite2D, it might have been unloaded or not set.");
    }

    Node2D::draw(renderer);
}

void Sprite2D::input(const InputManager* input) {
}
