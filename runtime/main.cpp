#include <SDL3/SDL_main.h>
#include "core/engine.h"



int main(int argc, char* argv[]) {

    if (!GEngine->initialize(1280, 720, "Golias Engine - Window")) {
        spdlog::error("Engine initialization failed, exiting");
        return -1;
    }

    auto& ecs = GEngine->get_world();

    create_material("green_metal", Material{
        .albedo = glm::vec3(0, 1.0f, 0),
        .metallic = 0.5f,
        .roughness = 0.5f
    });

    create_material("pink_emissive", Material{
        .albedo = glm::vec3(1.f, 0.f, 1.0),
        .metallic = 1.f,
        .roughness = 0.1f,
        .emissive = glm::vec3(1, 0, 0),
        .emissive_strength = 1.0f
    });

    create_material("yellow", Material{
        .albedo = glm::vec3(1.0f, 1.0f, 0)
    });

    create_material("cyan", Material{
        .albedo = glm::vec3(0.0f, 1.0f, 1.0)
    });

    create_material("ground_gray", Material{
        .albedo = glm::vec3(0.5f, 0.5f, 0.5f),
        .metallic = 0.0f,
        .roughness = 1.0f
    });

    create_material("red_rough", Material{
        .albedo = glm::vec3(1.f, 0.1f, 0.1f),
        .metallic = 0.0f,
        .roughness = 0.3f
    });

    create_material("green_shiny", Material{
        .albedo = glm::vec3(0.1f, 0.8f, 0.1f),
        .metallic = 0.9f,
        .roughness = 0.1f
    });

    create_material("blue_metal", Material{
        .albedo = glm::vec3(0.2f, 0.2f, 1.0f),
        .metallic = 0.3f,
        .roughness = 0.7f
    });

    create_material("dark_metal_ground", Material{
        .albedo = glm::vec3(0.1f, 0.1f, 0.1f),
        .metallic = 0.9f,
        .roughness = 0.9f
    });

    create_material("random_default", Material{
        .albedo = glm::vec3(0.5f, 0.5f, 0.5f),
        .metallic = 0.2f,
        .roughness = 0.8f
    });

    auto camera =
        ecs.entity("MainCamera")
           .set<Transform3D>({.position = {0, 2, 20}, .rotation = {-0.4f, 0, 0}})
           .add<Camera3D>();

    auto dirLight = ecs.entity()
      .set(Transform3D{})
      .set(DirectionalLight{
          glm::normalize(glm::vec3(1.0f, -2.5f, 1.0f)), // normalized sunlight direction
          glm::vec3(1.0f, 0.95f, 0.8f),                 // warm sunlight color
          1.0f,                                         // intensity
          true                                          // cast shadows
      });

    auto dirLight2 = ecs.entity()
        .set(Transform3D{})
        .set(DirectionalLight{
            glm::normalize(glm::vec3(-0.3f, -1.0f, 0.2f)), // fill/rim light
            glm::vec3(0.6f, 0.75f, 1.0f),                  // cool tint for contrast
            0.8f,                                          // weaker intensity
            false                                          // no shadow casting
        });


    auto spotLight1 = ecs.entity()
                         .set(Transform3D{glm::vec3(5, 5, 5)})
                         .set(SpotLight{
                             glm::vec3(-1, -1, -1),
                             glm::vec3(1.0f, 0.3f, 0.3f),
                             30.0f,
                             12.5f,
                             17.5f
                         });

    auto spotLight2 = ecs.entity()
                         .set(Transform3D{glm::vec3(-5, 5, 5)})
                         .set(SpotLight{
                             glm::vec3(1, -1, -1),
                             glm::vec3(0.3f, 0.3f, 1.0f),
                             50.0f,
                             12.5f,
                             17.5f
                         });


    // TODO: create api for lights and camera
    create_model_entity("dmg_helmet", "res://sprites/obj/DamagedHelmet.glb",
                       glm::vec3(10, 1, -5));

    create_model_entity("nagon", "res://sprites/obj/nagonford/Nagonford_Animated.glb",
                       glm::vec3(0, 0, 0));


    // create_model_entity("Sponza", "res://sprites/obj/sponza/Sponza.glb",
    //                      glm::vec3(0, -2, 0), glm::vec3(0), glm::vec3(0.1f));

    create_mesh_entity("Cylinder", "res://models/cylinder.obj",
                      glm::vec3(0, 0, -15), glm::vec3(0), glm::vec3(1.0f),
                      "green_metal");

    create_mesh_entity("Torus", "res://models/torus.obj",
                      glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(1.0f),
                      "pink_emissive");

    create_mesh_entity("Cone", "res://models/cone.obj",
                      glm::vec3(0, 0, 15), glm::vec3(0), glm::vec3(1.0f),
                      "yellow");

    create_mesh_entity("BlenderMonkey", "res://models/blender_monkey.obj",
                      glm::vec3(-10, 0, 10), glm::vec3(0), glm::vec3(1.0f),
                      "cyan");

    create_mesh_entity("Plane", "res://models/plane.obj",
                      glm::vec3(0, -0.5, 0), glm::vec3(0), glm::vec3(10.0f),
                      "ground_gray");


    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-100.0f, 100.0f);


    for (int i = 0; i < 1000; ++i) {
        float x = 30.0f + dist(rng);
        float y = 30.0f + dist(rng);
        float z = 30.0f + dist(rng);

        std::string name = "Cube_" + std::to_string(i);
        create_mesh_entity(
            name.c_str(),
            "res://models/cube.obj",
            glm::vec3(x, y, z),
            glm::vec3(0.0f),
            glm::vec3(1.0f),
            "blue_metal"
        );
    }


    create_mesh_entity("Red Cube", "res://models/cube.obj",
                      glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(1.5f),
                      "red_rough");

    create_mesh_entity("Metallic Sphere", "res://models/sphere.obj",
                      glm::vec3(-3, 1, 0), glm::vec3(0), glm::vec3(1.5f),
                      "green_shiny");

    create_mesh_entity("SmallCube", "res://models/cube.obj",
                      glm::vec3(-3, 5, 10), glm::vec3(0), glm::vec3(0.5f),
                      "blue_metal");

    create_mesh_entity("Ground", "res://models/cube.obj",
                      glm::vec3(0, -5, 0), glm::vec3(0), glm::vec3(1000.0f, 0.1f, 1000.0f),
                      "dark_metal_ground");

    GEngine->run();

    return 0;
}