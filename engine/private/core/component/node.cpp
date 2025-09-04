#include "core/component/node.h"


void Node2D::translate(float dx, float dy) {
    _transform.position += glm::vec2(dx, dy);
}

void Node2D::rotate(float radians) {
    _transform.rotation += radians;
}

void Node2D::scale(float sx, float sy) {
    _transform.scale *= glm::vec2(sx, sy);
}

Node2D::Node2D() = default;

void Node2D::set_z_index(int index) {
    if (_z_index == index) {
        return;
    }

    _z_index = index;
}


int Node2D::get_z_index() const {
    return _z_index;
}

int Node2D::get_effective_z_index() const {
    if (_parent) {
        return _parent->get_effective_z_index() + _z_index;
    }
    return _z_index;
}

void Node2D::print_tree(const int indent) const {
    for (int i = 0; i < indent; ++i) {
        printf(" ");
    }

    printf("%s (zIndex: %d, Position: %.2f, %.2f)\n", _name.c_str(), _z_index, _transform.position.x, _transform.position.y);

    for (const auto& [name, node] : _nodes) {
        node->print_tree(indent + 2);
    }
}

Transform2D Node2D::get_global_transform() const {
    if (_parent) {
        auto [position, scale, rotation] = _parent->get_global_transform();

        Transform2D global;
        global.position = position + _transform.position;
        global.scale    = scale * _transform.scale;
        global.rotation = rotation + _transform.rotation;

        return global;
    }

    return _transform;
}

Transform2D Node2D::get_transform() const {
    return _transform;
}

void Node2D::set_transform(const Transform2D& transform) {
    _transform = transform;
}


Node2D::~Node2D() {
    for (auto& [name, child] : _nodes) {
        if (child) {
            child->queue_free();
            delete child;
        }
    }

    _nodes.clear();
}

void Node2D::add_child(const std::string& base_name, Node2D* node) {
    std::string name = base_name;

    int index = 1;
    while (_nodes.contains(name)) {
        name = base_name + std::to_string(index);
        ++index;
    }

    node->_parent = this;
    node->_name   = name;

    _nodes.emplace(name, node);

    if (_is_ready && !node->_is_ready) {
        node->ready();
    }
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


const std::string& Node2D::get_name() const {
    return _name;
}

bool Node2D::is_visible() const {
    return _is_visible;
}

bool Node2D::is_effective_visible() const {
    if (!_is_visible) {
        return false;
    }
    if (_parent) {
        return _parent->is_effective_visible();
    }
    return true;
}

HashMap<std::string, Node2D*>& Node2D::get_tree() {
    return _nodes;
}

bool Node2D::is_alive() const {

    return !_to_free;
}

void Node2D::change_visibility(bool visible) {
    _is_visible = visible;
}

void Node2D::ready() {

    if (_is_ready) {
        return;
    }

    _is_ready = true;

    for (const auto& [name, child] : _nodes) {
        child->ready();
    }
}


void Node2D::process(double delta_time) {
    if (!is_effective_visible()) {
        return;
    }

    for (auto it = _nodes.begin(); it != _nodes.end();) {
        Node2D* child = it->second;
        child->process(delta_time);

        if (!child->is_alive()) {
            delete child;
            it = _nodes.erase(it);
        } else {
            ++it;
        }
    }
}

void Node2D::draw(Renderer* renderer) {
    if (!is_effective_visible() || !is_alive()) {
        return;
    }

    for (const auto& [name, child] : _nodes) {
        child->draw(renderer);
    }
}

void Node2D::input(const InputManager* input) {
    if (!is_alive() || !input) {
        return;
    }

    for (const auto& [name, child] : _nodes) {
        child->input(input);
    }
}


void Node2D::draw_inspector() {
#if !defined(WITH_EDITOR)
    return;
#else
    if (this != g_selected_node) {
        return;
    }

    ImGui::Text("Name: %s", _name.c_str());
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat2("Position", &_transform.position.x, 0.1f, -FLT_MAX, FLT_MAX, "%.2f");
        ImGui::DragFloat2("Scale", &_transform.scale.x, 0.1f, 0.0f, FLT_MAX, "%.2f");
        ImGui::DragFloat("Rotation", &_transform.rotation, 0.1f, -FLT_MAX, FLT_MAX, "%.2f");

        if (ImGui::Button("Reset")) {
            _transform.position = {0.0f, 0.0f};
            _transform.scale    = {1.0f, 1.0f};
            _transform.rotation = 0.0f;
        }
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Visible", &_is_visible);
        ImGui::InputInt("Z-Index", &_z_index);
        ImGui::Checkbox("Mark for Deletion", &_to_free);
    }

    // ImGui::Separator();
    //
    // if (!_nodes.empty()) {
    //     if (ImGui::CollapsingHeader("Children")) {
    //         for (const auto& [name, child] : _nodes) {
    //             if (ImGui::TreeNode(name.c_str())) {
    //                 child->draw_inspector();
    //                 ImGui::TreePop();
    //             }
    //         }
    //     }
    // }
#endif
}

void Node2D::draw_hierarchy() {
#if !defined(WITH_EDITOR)
    return;
#else
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    if (_nodes.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (this == g_selected_node) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    const bool node_open = ImGui::TreeNodeEx((void*) this, flags, _name.c_str());

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        g_selected_node = (g_selected_node == this) ? nullptr : this;
    }

    if (ImGui::BeginPopupContextItem(("NodeContext_" + _name).c_str())) {

        if (ImGui::MenuItem("Add child")) {
        }

        if (ImGui::MenuItem("Rename")) {
        }

        ImGui::BeginDisabled();

        if (ImGui::MenuItem("Remove")) {
            _to_free = true;
        }

        ImGui::EndDisabled();


        ImGui::EndPopup();
    }

    if (node_open) {
        for (auto& [name, child] : _nodes) {
            child->draw_hierarchy();
        }
        ImGui::TreePop();
    }

#endif
}

void Node2D::queue_free() {
    LOG_INFO("Node2D::free()");
    _to_free = true;
}
