#include "core/component/sprite_node.h"
#include "core/ember_core.h"


void SpriteNode::ready() {

}

void SpriteNode::process(double delta_time) {
}

void SpriteNode::draw(Renderer* renderer) {
    renderer->draw_texture(_texture, get_global_transform(), _size, _color);
    Node2D::draw(renderer);
}

void SpriteNode::event(const InputManager* input) {

}
