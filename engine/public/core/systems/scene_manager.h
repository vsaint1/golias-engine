#pragma once

#include "core/component/node.h"


class Scene {
public:
    Scene() = default;

    virtual void on_ready() {
    }

    virtual void on_input(InputManager* input) {
    }

    virtual void on_draw(Renderer* renderer) {
    }

    virtual void on_update(double delta_time) {
    }

    virtual void on_destroy() {
    }


    void setup();

    void update(double delta_time) ;

    void input(InputManager* input);

    void draw(Renderer* renderer);

    void destroy();

    [[nodiscard]] Node2D* get_root() const;
    void set_root(Node2D* node);

    virtual ~Scene();

    bool did_load = false;

protected:

    std::unique_ptr<Node2D> _root = std::make_unique<Node2D>("Root");
};


class SceneManager final : public EngineManager {
public:
    bool initialize() override;
    void shutdown() override;

    void update(double delta_time) override;

    void set_scene(const std::string& name);
    [[nodiscard]] Scene* get_scene(const std::string& name) const;

    void add_scene(const std::string& name, std::unique_ptr<Scene> scene);
    bool remove_scene(const std::string& name);

    ~SceneManager() override;
    Scene* _current = nullptr;

private:
    std::string _next_scene_name;

    void process_scene_change();

    HashMap<std::string, std::unique_ptr<Scene>> _scenes;
};
