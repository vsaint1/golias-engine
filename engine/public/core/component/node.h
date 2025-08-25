#pragma once
#include "core/engine_structs.h"
#include "core/input/input_manager.h"
#include "core/renderer/shader.h"
#include "helpers/logging.h"
#include "transform_node.h"

class Renderer;

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

    void rotate(float degrees);

    void scale(float sx, float sy);


    [[nodiscard]] Transform2D get_global_transform() const;

    [[nodiscard]] Transform2D get_transform() const;

    void set_transform(const Transform2D& transform);

    void add_child(const std::string& name, Node2D* node);

    Node2D* get_node(const std::string& path);

    template <typename T>
    T* get_node();

    virtual void ready();

    virtual void process(double delta_time);

    virtual void draw(Renderer* renderer);

    virtual void input(const InputManager* input);

    virtual ~Node2D();

protected:
    Transform2D _transform = {};

    std::string _name = "Node";

    int _z_index = 0;

    bool _is_visible = true;

private:
    Node2D* _parent = nullptr;

    HashMap<std::string, Node2D*> _nodes;
};


template <typename T>
T* Node2D::get_node() {
    for (auto& [name, node] : _nodes) {
        if (auto val = dynamic_cast<T*>(node)) {
            return val;
        }
    }

    LOG_WARN("Node of requested type %s was not found.", typeid(T).name());

    return nullptr;
}
