#pragma once
#include "core/engine_structs.h"
#include "transform.h"
#include "helpers/logging.h"

class Renderer;

class Node2D {
public:
    Node2D() = default;

    explicit Node2D(std::string name) : _name(std::move(name)) {
    }

    void set_z_index(int index);

    int get_z_index() const;

    void print_tree(int indent = 0) const;

    Transform get_global_transform() const;

    Transform get_transform() const;

    void set_transform(const Transform& transform);

    virtual ~Node2D();

    void add_child(const std::string& name, Node2D* node);

    Node2D* get_node(const std::string& path);

    virtual void process(double delta_time);

    virtual void draw(Renderer* renderer);

protected:
    Transform _transform = {};

    std::string _name = "Node";
    int _zIndex       = 0;

    Node2D* _parent = nullptr;

    HashMap<std::string, Node2D*> _nodes;
    std::vector<Node2D*> _draw_list;
};
