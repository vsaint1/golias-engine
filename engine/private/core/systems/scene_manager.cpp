#include "core/systems/scene_manager.h"

#include "core/engine.h"


void Scene::set_root(Node2D* node) {
    _root = std::unique_ptr<Node2D>(node);
}


void Scene::setup() {

    if (!_root) {
        _root = std::make_unique<Node2D>("Root");
    }

    if (!did_load) {
        did_load = true;
        on_ready();
        _root->ready();
    }
}

void Scene::update(double dt) {
    if (_root && did_load) {

        on_update(dt);
        _root->process(dt);
    }
}

void Scene::input(InputManager* input) {

    if (_root && did_load && input) {
        on_input(input);
        _root->input(input);
    }
}

void Scene::draw(Renderer* renderer) {
    if (!renderer) {
        return;
    }

    if (_root && did_load) {
        on_draw(renderer);
        _root->draw(renderer);
    }
}

void Scene::destroy() {
    on_destroy();

    _root    = nullptr;
    did_load = false;
}

Node2D* Scene::get_root() const {
    return _root.get();
}

Scene::~Scene() = default;

// ====================================================================================
bool SceneManager::initialize() {
    return true;
}

void SceneManager::shutdown() {
    _scenes.clear();
}

void SceneManager::update(double delta_time) {
    if (_current) {

        // CALLED ONCE
        // _current->setup();

        // CALLED EVERY FRAME
        _current->update(delta_time);

        _current->input(GEngine->input_manager());

        _current->draw(GEngine->get_renderer());
    }

    process_scene_change();
}


void SceneManager::set_scene(const std::string& name) {
    auto it = _scenes.find(name);
    if (it != _scenes.end()) {
        _next_scene_name = name;
    } else {
        LOG_WARN("Scene '%s' not found", name.c_str());
    }
}

void SceneManager::process_scene_change() {

    if (_next_scene_name.empty()) {
        return;
    }

    auto it = _scenes.find(_next_scene_name);
    if (it != _scenes.end()) {
        if (_current) {
            _current->destroy();
        }

        _current = it->second.get();
        if (_current) {
            _current->setup();
        }
    }

    _next_scene_name.clear();
}


void SceneManager::add_scene(const std::string& name, std::unique_ptr<Scene> scene) {
    _scenes[name] = std::move(scene);
}

Scene* SceneManager::get_scene(const std::string& name) const {
    auto it = _scenes.find(name);
    if (it != _scenes.end()) {
        return it->second.get();
    }

    return nullptr;
}

bool SceneManager::remove_scene(const std::string& name) {
    auto it = _scenes.find(name);
    if (it != _scenes.end()) {
        if (_current == it->second.get()) {
            _current->on_destroy();
            _current = nullptr;
        }
        _scenes.erase(it);
        return true;
    }

    return false;
}

SceneManager::~SceneManager() {
}
