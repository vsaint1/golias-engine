#include "core/engine.h"
#include <SDL3/SDL_main.h>


#define WINDOW_W 1280
#define WINDOW_H 720

int main(int argc, char* argv[]) {

    if (!GEngine->initialize(WINDOW_W, WINDOW_H)) {
        return SDL_APP_FAILURE;
    }


    auto ui_icons = GEngine->get_renderer()->load_texture("ui_icons", "res://ui/icons/icons_64.png");

    auto& world = GEngine->get_world();


    auto scene1 = world.entity("MenuScene").add<tags::Scene>().add<tags::ActiveScene>();
    auto scene2 = world.entity("GameScene").add<tags::Scene>();

    auto player = world.entity("Player")
                      .set<Transform2D>({{100, 100}, {1, 1}, 50, 1})
                      .set<Shape2D>({ShapeType::RECTANGLE, {0, 1, 0, 1}, true, {50, 50}})
                      .set<Script>({"res://scripts/test.lua"})
                      .child_of(scene1);

    auto& camera = world.entity("MainCamera").add<tags::MainCamera>().set<Camera2D>({})
    .set<Follow>({player, {0, -100,0}, 0.1f})
    .child_of(scene1);

    auto enemy = world.entity("Enemy")
                     .set<Transform2D>({{300, 200}, {1, 1}, 0})
                     .set<Shape2D>({ShapeType::CIRCLE, {1, 0, 0, 1}, false, {32, 32}, 32})
                     .child_of(scene1);

    auto enemy_sprite = world.entity("EnemySprite").set<Transform2D>({{0, 0}, {1, 1}, 0}).set<Sprite2D>({"ui_icons"}).child_of(enemy);

    auto enemy_label = world.entity("EnemyLabel").set<Transform2D>({{0, -20}, {1, 1}, 0}).set<Label2D>({"Enemy 💀"}).child_of(enemy);


    // FileAccess("user://scenes/test.json",ModeFlags::WRITE).store_string(world.to_json().c_str());


    GEngine->run();

    return 0;
}
