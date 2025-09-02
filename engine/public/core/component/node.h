#pragma once
#include "../systems/input_manager.h"
#include "../systems/logging_sys.h"
#include "core/engine_structs.h"
#include "core/renderer/shader.h"
#include "transform_node.h"

enum class ShapeType {
    RECTANGLE,
    CIRCLE,
    POLYGON,
    CAPSULE

};
class Renderer;

/**
 * @brief 2D Node base class for scene graph.
 *
 * @details
 * - Supports hierarchical transformations (position, rotation, scale).
 * - Manages child nodes and parent-child relationships.
 * - Handles visibility and z-index for rendering order.
 * - Provides lifecycle methods: ready(), process(), draw(), input().
 *
 * @version 0.5.0
 *
 */
class Node2D {
public:
    Node2D();

    explicit Node2D(std::string name) : _name(std::move(name)) {
    }

    void set_z_index(int index);

    [[nodiscard]] int get_z_index() const;

    [[nodiscard]] int get_effective_z_index() const;

    void print_tree(int indent = 0) const;

    void change_visibility(bool visible);

    void translate(float dx, float dy);

    void rotate(float radians);

    void scale(float sx, float sy);


    [[nodiscard]] Transform2D get_global_transform() const;

    [[nodiscard]] Transform2D get_transform() const;

    void set_transform(const Transform2D& transform);

    void add_child(const std::string& base_name, Node2D* node);

    Node2D* get_node(const std::string& path);

    template <typename T>
    T* get_node();

    virtual void ready();

    virtual void process(double delta_time);

    virtual void draw(Renderer* renderer);

    virtual void input(const InputManager* input);

    void queue_free();

    virtual ~Node2D();

    [[nodiscard]] const std::string& get_name() const;

    [[nodiscard]] bool is_visible() const;

    [[nodiscard]] bool is_effective_visible() const;

    HashMap<std::string, Node2D*>& get_tree();

    bool is_alive() const;

protected:
    Transform2D _transform = {};

    std::string _name = "Node";

    int _z_index = 0;

    bool _is_visible = true;

    bool _is_ready = false;

    bool _to_free = false;
private:
    Node2D* _parent = nullptr;
    HashMap<std::string, Node2D*> _nodes;
};


template <typename T>
T* Node2D::get_node() {
    if (auto self_as_t = dynamic_cast<T*>(this)) {
        return self_as_t;
    }

    for (auto& [name, child] : _nodes) {
        if (auto val = dynamic_cast<T*>(child)) {
            return val;
        }
    }

    for (auto& [name, child] : _nodes) {
        if (auto val = child->get_node<T>()) {
            return val;
        }
    }

    LOG_WARN("Node of requested type %s was not found.", typeid(T).name());

    return nullptr;
}
