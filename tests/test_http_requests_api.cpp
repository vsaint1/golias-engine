#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1280;
int SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {
    if (!GEngine->initialize(SCREEN_WIDTH, SCREEN_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return -1;
    }

    Audio* bgm  = Audio::load("sounds/lullaby.mp3", "Music");
    Audio* coin_sfx = Audio::load("sounds/coin.wav", "SFX");

    AudioBus& music_bus = AudioBus::get_or_create("Music");

    bgm->play();


    // Input
    InputAction move_left("move_left");
    move_left.bind_key(SDL_SCANCODE_A).bind_key(SDL_SCANCODE_LEFT).bind_gamepad_button(SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    GEngine->input_manager()->register_action(move_left);

    InputAction move_right("move_right");
    move_right.bind_key(SDL_SCANCODE_D).bind_key(SDL_SCANCODE_RIGHT).bind_gamepad_button(SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    GEngine->input_manager()->register_action(move_right);

    // Assets
    auto player_texture = GEngine->get_renderer()->load_texture("sprites/Character_001.png");
    auto enemy_texture  = GEngine->get_renderer()->load_texture("sprites/Character_002.png");
    GEngine->get_renderer()->load_font("fonts/Minecraft.ttf", "mine", 32);

    // Scene roots
    Node2D* world_root = new Node2D("World");
    Node2D* ui_root    = new Node2D("UI");


    CollisionShape2D* player_shape = new CollisionShape2D();
    player_shape->type             = ShapeType::RECTANGLE;
    player_shape->size             = {32, 32};
    player_shape->translate(16, 16);


    CollisionShape2D* enemy_shape = new CollisionShape2D();
    enemy_shape->type             = ShapeType::RECTANGLE;
    enemy_shape->size             = {32, 32};
    enemy_shape->translate(16, 16);


    // Player
    Node2D* player = new Node2D("Player");
    player->set_transform({{150.f, 100.f}, {1, 1}, 0.f});
    Sprite2D* player_sprite = new Sprite2D(player_texture);
    player_sprite->set_region({0, 0, 32, 32}, {64, 64});
    player->add_child("Image", player_sprite);
    player->add_child("CollisionPlayer", player_shape);


    auto test_log_fail = player->get_node<Circle2D>();

    // Enemy
    Node2D* enemy = new Node2D("Enemy");
    enemy->set_transform({{600.f, 100.f}, {1, 1}, 0.f});
    Sprite2D* enemy_sprite = new Sprite2D(enemy_texture);
    enemy_sprite->set_region({0, 0, 32, 32}, {64, 64});
    enemy->add_child("Image", enemy_sprite);
    enemy->add_child("CollisionEnemy", enemy_shape);


    // Camera
    Camera2D* camera = new Camera2D("Camera");
    camera->follow(player);
    camera->set_viewport(320, 180);
    world_root->add_child("Camera", camera);

    // UI Label
    Label* status = new Label("mine", "golias_bento");
    status->set_text("Catch the enemy!");
    status->set_transform({{20, 50}, {1, 1}, 0.f});
    status->set_shadow(true);

    ui_root->add_child("Status", status);

    world_root->add_child("Player", player);
    world_root->add_child("Enemy", enemy);

    world_root->ready();
    ui_root->ready();

    SDL_Event e;

    player_shape->on_body_entered([&](const CollisionShape2D* collided) {
        status->set_text("You Collided with [color=#00FF00]%s[/color]", collided->get_name().c_str());
        status->set_font_size(16);
        coin_sfx->play();

    });

    player_shape->on_body_exited([&](const CollisionShape2D* _) {
        status->set_text("Catch the enemy!");
        status->set_font_size(32);
    });

    while (GEngine->is_running) {
        while (SDL_PollEvent(&e)) {
            GEngine->input_manager()->process_event(e);
        }

        float dt = GEngine->time_manager()->get_delta_time();

        GEngine->update(dt);

        world_root->process(dt);
        ui_root->process(dt);

        if (bool game_won = false; !game_won) {
            if (GEngine->input_manager()->is_action_pressed("move_right")) {
                player->translate(200.f * dt, 0);
            }

            if (GEngine->input_manager()->is_action_pressed("move_left")) {
                player->translate(-200.f * dt, 0);
            }
        }

        GEngine->get_renderer()->clear();

        GEngine->get_renderer()->set_view_matrix(camera->get_view_matrix());
        world_root->draw(GEngine->get_renderer());
        GEngine->get_renderer()->flush(); // flush World

        // TODO: create a Control node that ignores camera
        GEngine->get_renderer()->set_view_matrix();
        ui_root->draw(GEngine->get_renderer());
        GEngine->get_renderer()->flush(); // flush UI


        GEngine->get_renderer()->present();
    }

    delete world_root;
    delete ui_root;
    GEngine->shutdown();
    return 0;
}
