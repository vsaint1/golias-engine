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

    const int entities_count = 10000;

    for (int i = 0; i < entities_count; i++) {
        float angle  = (float) i / (float) entities_count * 360.0f;
        float radius = 50.0f + (rand() % 100);
        float x      = cos(glm::radians(angle)) * radius;
        float z      = sin(glm::radians(angle)) * radius;
        float y      = 0.0f;

        auto entity = GEngine->get_world().entity();

       
        const auto name  = "entity_" + std::to_string(i);
        entity.set_name(name.c_str());

        glm::vec3 color = glm::vec3(random_number<float>(0.0f, 1.0f), random_number<float>(0.0f, 1.0f), random_number<float>(0.0f, 1.0f));
        glm::vec3 size = glm::vec3(random_number<float>(0.5f, 5.0f));
        entity.set<MeshInstance3D>({.size = size, .color = color});
     
        entity.set<Transform3D>({.position = {x, y, z}, .rotation = {0, (float) (rand() % 360), 0}, .scale = {1.0f, 1.0f, 1.0f}});
        entity.child_of(scene);
    }


    GEngine->run();

    return 0;
}
