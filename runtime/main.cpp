#include <SDL3/SDL_main.h>
#include "core/engine.h"

int main(int argc, char* argv[]) {


    if (!GEngine->initialize(1280, 720, "Golias Engine - Window")) {
        spdlog::error("Engine initialization failed, exiting");
        return -1;
    }

    auto& ecs = GEngine->get_world();

    auto renderer = GEngine->get_renderer();

    // Create camera
    auto camera =
    ecs.entity("MainCamera").set<Transform3D>({.position = {0, 2, 20}, .rotation = {-0.4f, 0, 0}}).add<Camera3D>();

    // Create directional light (sun) with shadows
    auto dirLight = ecs.entity()
                       .set(Transform3D{})
                       .set(DirectionalLight{glm::vec3(1, -2.5, 1), glm::vec3(1.0f, 0.95f, 0.8f), 2.0f, true});

    // Create spot lights (no shadows)
    auto spotLight1 = ecs.entity()
                         .set(Transform3D{glm::vec3(5, 5, 5)})
                         .set(SpotLight{glm::vec3(-1, -1, -1), glm::vec3(1.0f, 0.3f, 0.3f), 30.0f, 12.5f, 17.5f});

    auto spotLight2 = ecs.entity()
                         .set(Transform3D{glm::vec3(-5, 5, 5)})
                         .set(SpotLight{glm::vec3(1, -1, -1), glm::vec3(0.3f, 0.3f, 1.0f), 50.0f, 12.5f, 17.5f});


    Model carModel = ObjectLoader::load_model("res/sprites/obj/Car2.obj", renderer);
    for (size_t i = 0; i < carModel.meshes.size(); i++) {
        ecs.entity(("Car_Mesh_" + std::to_string(i)).c_str())
           .set(Transform3D{glm::vec3(-10, 0, -5), glm::vec3(0), glm::vec3(1.0f)})
           .set(carModel.meshes[i])
           .set(carModel.materials[i]);
    }


    Model damagedHelmet = ObjectLoader::load_model("res/sprites/obj/DamagedHelmet.glb", renderer);
    for (size_t i = 0; i < damagedHelmet.meshes.size(); i++) {
        ecs.entity(("Helmet_Mesh_" + std::to_string(i)).c_str())
           .set(Transform3D{glm::vec3(15, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
           .set(damagedHelmet.meshes[i])
           .set(damagedHelmet.materials[i]);
    }

    // Model sponza = ObjectLoader::load_model("res/sprites/obj/sponza/sponza.glb", renderer);
    // for (size_t i = 0; i < sponza.meshes.size(); i++) {
    //     ecs.entity(("Sponza_Mesh_" + std::to_string(i)).c_str())
    //         .set(Transform3D{glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
    //         .set(sponza.meshes[i])
    //         .set(sponza.materials[i]);
    // }

    Model nagonford = ObjectLoader::load_model("res/sprites/obj/nagonford/Nagonford_Animated.glb", renderer);
    for (size_t i = 0; i < nagonford.meshes.size(); i++) {
        ecs.entity(("Nagonford_Mesh_" + std::to_string(i)).c_str())
           .set(Transform3D{glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1.0f)})
           .set(nagonford.meshes[i])
           .set(nagonford.materials[i]);
    }

    MeshInstance3D cylinderMesh = ObjectLoader::load_mesh("res/models/cylinder.obj");
    ecs.entity("Cylinder").set(Transform3D{glm::vec3(0, 0, -10), glm::vec3(0), glm::vec3(1.0f)}).set(cylinderMesh).set(Material{
        .albedo = glm::vec3(0, 0, 1.0f)});

    MeshInstance3D torusMesh = ObjectLoader::load_mesh("res/models/torus.obj");
    ecs.entity("Torus").set(Transform3D{glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(1.0f)}).set(torusMesh).set(Material{
        .albedo = glm::vec3(1.f, 0.f, 1.0), .metallic = 1.f, .roughness = 0.1, .emissive = glm::vec3(1, 0, 0), .emissiveStrength = 1.0f});

    MeshInstance3D coneMesh = ObjectLoader::load_mesh("res/models/cone.obj");
    ecs.entity("Cone").set(Transform3D{glm::vec3(0, 0, 20), glm::vec3(0), glm::vec3(1.0f)}).set(coneMesh).set(Material{
        .albedo = glm::vec3(0, 1.0f, 1.0)});

    MeshInstance3D blenderMonkeyMesh = ObjectLoader::load_mesh("res/models/blender_monkey.obj");
    ecs.entity("Cone").set(Transform3D{glm::vec3(0, 0, 10), glm::vec3(0), glm::vec3(1.0f)}).set(blenderMonkeyMesh).set(Material{
        .albedo = glm::vec3(1.0f, 0.0f, 1.0)});

    MeshInstance3D cubeMesh = ObjectLoader::load_mesh("res/models/cube.obj");
    ecs.entity("Red Cube")
       .set(Transform3D{glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(1.5f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(0.8f, 0.1f, 0.1f), 0.0f, 0.3f, 1.0f});

    MeshInstance3D sphereMesh = ObjectLoader::load_mesh("res/models/sphere.obj");

    ecs.entity("Sphere")
       .set(Transform3D{glm::vec3(-3, 0, 0), glm::vec3(0), glm::vec3(1.5f)})
       .set(sphereMesh)
       .set(Material{glm::vec3(1.f), 0.0f, 0.1f, 1.0f});

    ecs.entity("Cube")
       .set(Transform3D{glm::vec3(3, 0, 0), glm::vec3(0), glm::vec3(1.f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(1.f), 0.0f, 0.0f, 1.0f,});

    ecs.entity("Ground")
       .set(Transform3D{glm::vec3(0, -2, 0), glm::vec3(0), glm::vec3(20.0f, 0.1f, 20.0f)})
       .set(cubeMesh)
       .set(Material{glm::vec3(1.0f), 0.0f, 0.8f, 1.0f});

    GEngine->run();

    return 0;
}
