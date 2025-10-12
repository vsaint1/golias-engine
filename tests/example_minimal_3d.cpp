#include "core/engine.h"
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
    const int WINDOW_W = 1280, WINDOW_H = 720;

    if (!GEngine->initialize(WINDOW_W, WINDOW_H)) {
        LOG_INFO("Failed to initialize engine");
        return -1;
    }


    auto scene = GEngine->get_world().entity("MainScene").add<tags::Scene>().add<tags::ActiveScene>();

    auto player = GEngine->get_world()
                      .entity("Player")
                      .set<MeshInstance3D>({.size = {1, 2, 1}, .color = {1, 0, 0}})
                      .set<Transform3D>({.position = {0, 2, 0}, .scale = {1, 1, 1}})
                      .set<Script>({"res://scripts/test.lua"})
                      .child_of(scene);

    auto camera  = GEngine->get_world()
                      .entity("MainCamera")
                      .set<Transform3D>({.position = {0, 5, 10}, .rotation = {-0.4f, 0, 0}})
                      .add<Camera3D>()
                      .child_of(player);


    auto e3 = GEngine->get_world()
                  .entity("car")
                  .set<Model>({.path = "sprites/obj/Car.obj"})
                  .set<Transform3D>({.position = {10, 2, 0}})
                  .child_of(scene);


    auto plane = GEngine->get_world()
                     .entity("plane")
                     .set<MeshInstance3D>({.size = {100, 0.1f, 100}, .color = {0.3f, 0.8f, 0.3f}})
                     .set<Transform3D>({.position = {0, 0, 0}})
                     .child_of(scene);

    GEngine->run();

    return 0;
}
