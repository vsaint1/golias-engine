#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1366;
int SCREEN_HEIGHT = 768;


int main(int argc, char* argv[]) {

    if (!GEngine->initialize("Node System", SCREEN_WIDTH, SCREEN_HEIGHT, OPENGL)) {
        return -1;
    }

    auto player_texture = GEngine->get_renderer()->load_texture("sprites/Character_001.png");
    auto enemy_texture  = GEngine->get_renderer()->load_texture("sprites/Character_002.png");
    auto mine_font      = GEngine->get_renderer()->load_font("fonts/Minecraft.ttf", 32);

    Node2D* root = new Node2D("Root");

    Node2D* player = new Node2D();
    player->set_z_index(5);
    player->set_transform({glm::vec2(150.f, 100.f), glm::vec2(1.f), 0.0f});

    std::vector<glm::vec2> hexagon;

    for (int i = 0; i < 6; ++i) {
        const float angle = glm::radians(60.0f * i);
        glm::vec2 point   = glm::vec2(cos(angle), sin(angle)) * 50.f;
        hexagon.push_back(point);
    }

    Polygon2D* polygon = new Polygon2D(hexagon, true);
    polygon->translate(200, 100);


    Label* name = new Label(mine_font, "golias_bento", 32);
    name->set_text("golias bento %d", 123);
    name->set_z_index(10);

    Sprite2D* player_sprite = new Sprite2D(player_texture);
    player_sprite->set_region({0, 0, 32, 32}, glm::vec2(128));
    player_sprite->translate(100, 50);

    // sprite->change_visibility(false);

    Node2D* enemy = new Node2D();
    enemy->set_z_index(4);
    enemy->set_transform({glm::vec2(600.f, 100.f), glm::vec2(1.f), 320.0f});

    Sprite2D* enemy_sprite = new Sprite2D(enemy_texture);

    Circle2D* circle = new Circle2D();

    root->add_child("Player", player);
    root->add_child("Enemy", enemy);

    if (Node2D* playerT = root->get_node("Player")) {
        playerT->print_tree();
    }

    player->add_child("Image", player_sprite);
    player->add_child("CollisionShape", polygon);
    player->add_child("Name", name);

    enemy->add_child("Image", enemy_sprite);
    enemy->add_child("CollisionShape", circle);

    root->print_tree();

    root->ready();

    SDL_Event e;
    while (GEngine->bIsRunning) {
        while (SDL_PollEvent(&e)) {

            GEngine->input_manager()->update();
            GEngine->time_manager()->update();

            GEngine->get_renderer()->clear_background({120, 100, 100, 255});

            GEngine->input_manager()->process(&e);

            root->event(GEngine->input_manager());

            GEngine->get_renderer()->begin_drawing();

            root->draw(GEngine->get_renderer());

            GEngine->get_renderer()->end_drawing();

            GEngine->time_manager()->fixed_frame_rate();
        }
    }

    delete root;

    GEngine->shutdown();

    return 0;
}
