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

    Node2D* root = new Node2D("Root");

    Node2D* player = new Node2D();
    player->set_z_index(5);
    player->set_transform({glm::vec2(150.f, 100.f), glm::vec2(1.f), 50.0f});

    SpriteNode* sprite = new SpriteNode(player_texture);
    player->add_child("Image", sprite);

    Node2D* enemy = new Node2D();
    enemy->set_z_index(4);
    enemy->set_transform({glm::vec2(600.f, 100.f), glm::vec2(1.f), 0.0f});

    SpriteNode* enemy_sprite = new SpriteNode(enemy_texture);
    enemy->add_child("Image", enemy_sprite);

    root->add_child("Player", player);
    root->add_child("Enemy", enemy);

    if (Node2D* playerT = root->get_node("Player")) {
        playerT->print_tree();
    }

    root->print_tree();

    SDL_Event e;
    while (GEngine->bIsRunning) {
        while (SDL_PollEvent(&e)) {

            GEngine->input_manager()->update();
            GEngine->time_manager()->update();

            GEngine->get_renderer()->clear_background({120, 100, 100, 255});

            GEngine->input_manager()->process(&e);

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
