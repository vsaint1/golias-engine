#include "core/api/engine_api.h"

#include "core/engine.h"


void change_scene(const std::string& scene_name) {
    auto& world = GEngine->get_world();
    world.entity().set<SceneChangeRequest>({scene_name});
}


flecs::entity get_entity_by_name(const std::string& name) {
    auto& world = GEngine->get_world();
    return world.lookup(name.c_str());
}

bool entity_has_component(flecs::entity& e, const std::string& component_name) {
    auto& world             = GEngine->get_world();
    flecs::entity component = world.lookup(component_name.c_str());
    if (!component.is_valid()) {
        return false;
    }
    return e.has(component);
}

bool entity_add_component(flecs::entity& e, const std::string& component_name) {
    auto& world             = GEngine->get_world();
    flecs::entity component = world.lookup(component_name.c_str());
    if (!component.is_valid()) {
        return false;
    }
    e.add(component);
    return true;
}

void entity_remove_component(flecs::entity& e, const std::string& component_name) {
    auto& world             = GEngine->get_world();
    flecs::entity component = world.lookup(component_name.c_str());

    if (!component.is_valid()) {
        return;
    }

    e.remove(component);
}

bool entity_is_valid(flecs::entity& e) {
    return e.is_valid();
}
