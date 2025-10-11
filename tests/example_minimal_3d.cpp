#include "core/engine.h"
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
    const int WINDOW_W = 1280, WINDOW_H = 720;

    if (!GEngine->initialize(WINDOW_W, WINDOW_H)) {
        LOG_INFO("Failed to initialize engine");
        return -1;
    }


    auto scene = GEngine->get_world().entity("MainScene").add<tags::Scene>().add<tags::ActiveScene>();

    Camera3D cam;
    cam.position = glm::vec3(0, 10, 20);
    auto camera  = GEngine->get_world().entity("MainCamera").set<Camera3D>(cam).add<tags::MainCamera>()
    .set<Script>({"res://scripts/test.lua"})
    .child_of(scene);



    auto e3 =
        GEngine->get_world().entity("car").set<Model>({.path = "sprites/obj/Car.obj"}).set<Transform3D>({.position = {10, 1, 0}}).child_of(scene);



    auto plane = GEngine->get_world().entity("plane").set<Cube>({.size = {100, 0.1f, 100}, .color = {0.3f, 0.8f, 0.3f}})
                     .set<Transform3D>({.position = {0, 0, 0}})
                     .child_of(scene);

    GEngine->run();

    return 0;
}
