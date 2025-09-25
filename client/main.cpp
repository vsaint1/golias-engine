#include "core/engine.h"
#include <SDL3/SDL_main.h>

#define WINDOW_W 1280
#define WINDOW_H 720

int main(int argc, char* argv[]) {

    if (!GEngine->initialize(WINDOW_W, WINDOW_H, "nullptr")) {
        return SDL_APP_FAILURE;
    }


    // TODO: We should get the entities from scene file (just test for now)
    auto player = GEngine->get_world()
                      .entity("Player")
                      .set<Transform2D>({{100, 100}, {1, 1}, 50})
                      .set<Shape>({ShapeType::RECTANGLE, {0, 1, 0, 1}, true, {50, 50}})
                      .set<Script>({"scripts/test.lua"});


    auto enemy = GEngine->get_world()
                     .entity("Enemy")
                     .set<Transform2D>({{300, 200}, {1, 1}, 0})
                     .set<Label2D>({"Enemy 🔥", {-5, -20}, {1, 1, 1, 1}})
                     .set<Shape>({ShapeType::CIRCLE, {1, 0, 0, 1}, false, {32, 32}, 20});


    LOG_INFO("Player %s", player.to_json().c_str());

    // LOG_INFO("World %s",GEngine->get_world().to_json().c_str());

    // FileAccess scene("user://scenes/test_scene.json", ModeFlags::WRITE);
    // scene.store_string(GEngine->get_world().to_json().c_str());
    GEngine->run();


    return 0;
}
