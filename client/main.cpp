#include "core/engine.h"
#include <SDL3/SDL_main.h>

#define WINDOW_W 1280
#define WINDOW_H 720



int main(int argc, char* argv[]) {

    if (!GEngine->initialize(WINDOW_W, WINDOW_H)) {
        return SDL_APP_FAILURE;
    }


    auto& world = GEngine->get_world();

    auto scene1 = world.entity("MenuScene").add<Scene>().add<ActiveScene>();
    auto scene2 = world.entity("GameScene").add<Scene>();

    auto player = world.entity("Player")
                      .set<Transform2D>({{100, 100}, {1, 1}, 50})
                      .set<Shape>({ShapeType::RECTANGLE, {0, 1, 0, 1}, true, {50, 50}})
                      .set<Script>({"scripts/test.lua"})
                      .child_of(scene1);

    auto enemy = world.entity("Enemy")
                     .set<Transform2D>({{300, 200}, {1, 1}, 0})
                     .set<Label2D>({"Enemy 🔥", {-5, -20}, {1, 1, 1, 1}})
                     .set<Shape>({ShapeType::CIRCLE, {1, 0, 0, 1}, false, {32, 32}, 20})
                     .child_of(scene2);

    // FileAccess("user://scenes/test.json",ModeFlags::WRITE).store_string(world.to_json().c_str());


    GEngine->run();

    return 0;
}
