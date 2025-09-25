#include "core/api/engine_api.h"
#include "core/engine.h"


void change_scene(const std::string& scene_name) {
    auto& world = GEngine->get_world();
    world.entity().set<SceneChangeRequest>({scene_name});
}
