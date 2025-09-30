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
    auto camera  = GEngine->get_world().entity("MainCamera").set<Camera3D>(cam).add<tags::MainCamera>().child_of(scene);

    auto map      = GEngine->get_renderer()->load_model("sprites/obj/tree_b_green.obj");
    auto campfire = GEngine->get_renderer()->load_model("sprites/obj/campfire_complete.obj");
    auto car2     = GEngine->get_renderer()->load_model("sprites/obj/Car2.obj");
    auto car      = GEngine->get_renderer()->load_model("sprites/obj/Car.obj");
    auto dragon   = GEngine->get_renderer()->load_model("sprites/obj/dragon.fbx");

    auto e1 = GEngine->get_world()
                  .entity("campfire")
                  .set<std::shared_ptr<Model>>(campfire)
                  .set<Transform3D>({.position = {0, 0, 0}})
                  .child_of(scene);

    auto e2 =
        GEngine->get_world().entity("car2").set<std::shared_ptr<Model>>(car2).set<Transform3D>({.position = {5, 1, 0}}).child_of(scene);

    auto e3 =
        GEngine->get_world().entity("car").set<std::shared_ptr<Model>>(car).set<Transform3D>({.position = {10, 1, 0}}).child_of(scene);

    auto e4 = GEngine->get_world()
                  .entity("dragon")
                  .set<std::shared_ptr<Model>>(dragon)
                  .set<Transform3D>({.position = {15, 0, 0}, .rotation = {-90, 0, 0}})
                  .child_of(scene);


    auto e5 = GEngine->get_world().entity("map").set<std::shared_ptr<Model>>(map).set<Transform3D>({.position = {-10, -5, 0}});

    GEngine->run();

    return 0;
}
