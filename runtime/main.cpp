#include "core/engine.h"
#include <SDL3/SDL_main.h>


int main(int argc, char* argv[]) {
    const int WINDOW_W = 1280, WINDOW_H = 720;

    if (!GEngine->initialize(WINDOW_W, WINDOW_H)) {
        LOG_INFO("Failed to initialize engine");
        return -1;
    }


    auto& world = GEngine->get_world();


    auto scene = world.entity("MainScene").add<tags::Scene>().add<tags::ActiveScene>();

    auto player = world.entity("Player")
                      .set<MeshInstance3D>({.size = {1, 2, 1}, .material = {.albedo = {0.8f, 0.1f, 0.1f}}})
                      .set<Transform3D>({.position = {0, 2, 30}, .scale = {1, 1, 1}})
                      .set<Script>({"res://scripts/test.lua"})
                      .child_of(scene);

    auto upper_plane =
        world.entity("upper_plane")
            .set<MeshInstance3D>({.size = {10, 1, 10}, .material = {.ambient = {0.1f, 0.1f, 0.1f}, .albedo = {1.f, 1.f, 1.f}}})
            .set<Transform3D>({.position = {25, 10, 0}})
            .child_of(scene);


    auto m1 = world
                  .entity()
                  .set<Model>({.path = "res://sprites/obj/DamagedHelmet.glb"})
                  .set<Transform3D>({.position = {-10, 2, 30}, .rotation = {0, 180, 0}, .scale = {1, 1, 1}})
                  .child_of(scene);
    auto camera =
        world.entity("MainCamera").set<Transform3D>({.position = {0, 5, 10}, .rotation = {-0.4f, 0, 0}}).add<Camera3D>().child_of(player);


    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            auto godette = world.entity()
                               .set<Model>({.path = "res://sprites/obj/godette/godette.glb"})
                               .set<Transform3D>({.position = {-10 + i * 2, 0, -5 + j * 2}, .rotation = {0, randf() * 360, 0}})
                               .set<Animation3D>({.current_animation = 23})
                               .child_of(scene);
        }
    }


    auto nagon = world.entity("Nagon")
                     .set<Model>({.path = "res://sprites/obj/nagonford/Nagonford_Animated.glb"})
                     .set<Transform3D>({.position = {0, 2.5f, -10}, .scale = {5, 5, 5}})
                     .set<Animation3D>({.current_animation = 43})
                     .child_of(scene);

    auto barrel = world.entity()
                      .set<Model>({.path = "res://sprites/obj/wine_barrel_01_4k.obj"})
                      .set<Transform3D>({.position = {0, 0, 5}})
                      .child_of(scene);

    auto sponza = world.entity()
                      .set<Model>({.path = "res://sprites/obj/sponza.obj"})
                      .set<Transform3D>({.position = {0, 0, 5}})
                      .child_of(scene);

    // for (int i = 0; i < 25; i++) {
    //     float x = -15.0f + i * 3.0f;
    //     float z = -15.0f + (i % 5) * 6.0f;

    //     float y = glm::min(20.0f, std::abs(std::sin(i * 0.5f) * 80.0f));

    //     float yaw   = glm::radians(20.0f * i);
    //     float pitch = glm::radians(std::sin(i * 0.3f) * 15.0f);
    //     float roll  = glm::radians(std::cos(i * 0.7f) * 360.0f);

    //     auto dragon = world.entity()
    //                        .set<Model>({ .path = "res://sprites/obj/dragon.fbx" })
    //                        .set<Transform3D>({
    //                            .position = { x, y, z },
    //                            .rotation = { pitch, yaw, roll }
    //                        })
    //                        .child_of(scene);
    // }
    auto car = world.entity().set<Model>({.path = "res://sprites/obj/Car.obj"}).set<Transform3D>({.position = {-40, 0, 0}}).child_of(scene);


    auto plane = world.entity("plane")
                     .set<MeshInstance3D>({.size     = {100, 0.1f, 100},
                                           .material = {.ambient  = {0.1f, 0.1f, 0.1f},
                                                        .albedo   = {0.3f, 1.f, 0.3f},
                                                        .metallic = {.specular = {1.f, 1.f, 1.f}, .value = 1.0f}}})
                     .set<Transform3D>({.position = {0, 0, 0}})
                     .child_of(scene);

    GEngine->run();

    return 0;
}
