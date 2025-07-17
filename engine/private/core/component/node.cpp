#include "core/component/node.h"


void Node2D::set_z_index(int index) {
    _zIndex = index;
}

int Node2D::get_z_index() const {
    return _zIndex;
}

void Node2D::print_tree(const int indent) const {
    for (int i = 0; i < indent; ++i) {
        printf(" ");
    }

    printf("%s (zIndex: %d, Position: %.2f, %.2f)\n", _name.c_str(), _zIndex, _transform.position.x, _transform.position.y);

    for (const auto&[name,node] : _nodes) {
        node->print_tree(indent + 2);
    }
}

Transform Node2D::get_global_transform() const {
    if (_parent) {
        auto [position, scale, rotation] = _parent->get_global_transform();

        Transform global;
        global.position = position + _transform.position;
        global.scale    = scale * _transform.scale;
        global.rotation = rotation + _transform.rotation;

        return global;
    }

    return _transform;
}

Transform Node2D::get_transform() const {
    return _transform;
}

void Node2D::set_transform(const Transform& transform) {
    _transform = transform;
}


Node2D::~Node2D() {
    for (auto it = _nodes.begin(); it != _nodes.end(); ++it) {
        delete it->second;
    }
}

void Node2D::add_child(const std::string& name, Node2D* node) {
    node->_parent = this;
    node->_name   = name;

    _nodes.emplace(name, node);

    _draw_list.push_back(node);
}

Node2D* Node2D::get_node(const std::string& path) {
    const size_t slash = path.find('/');


    const std::string head = (slash == std::string::npos) ? path : path.substr(0, slash);
    const std::string tail = (slash == std::string::npos) ? "" : path.substr(slash + 1);

    const auto it = _nodes.find(head);
    if (it == _nodes.end()) {
        LOG_ERROR("Node with name %s wasn't found", head.c_str());
        return nullptr;
    }

    if (tail.empty()) {
        return it->second;
    }

    return it->second->get_node(tail);
}

void Node2D::ready() {

}

void Node2D::process(double delta_time) {
    for (auto* child : _draw_list) {
        child->process(delta_time);
    }
}

void Node2D::draw(Renderer* renderer) {
    std::ranges::sort(_draw_list, [](const Node2D* a, const Node2D* b) { return a->_zIndex < b->_zIndex; });

    for (auto* child : _draw_list) {
        child->draw(renderer);
    }
}

void Node2D::event(const InputManager* input) {
    for (auto& [name, child] : _nodes) {
        child->event(input);
    }
}
