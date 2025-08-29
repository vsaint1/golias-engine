#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

int WINDOW_WIDTH  = 1280;
int WINDOW_HEIGHT = 720;

struct GameContext {
    Node2D* root;
    RigidBody2D* player;
    Renderer* renderer;
    Label* colliding;
    bool quit;
};

GameContext ctx;
SDL_Event e;
const float MOVE_SPEED = 200.0f; // px/s
const float JUMP_FORCE = 10.0f; // px impulse

void game_update() {
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#else
            ctx.quit = true;
#endif
        }
    }


    const bool* state = SDL_GetKeyboardState(nullptr);

    const double dt = GEngine->time_manager()->get_delta_time();

    glm::vec2 vel = ctx.player->get_velocity();

    if (state[SDL_SCANCODE_A]) vel.x -= MOVE_SPEED * dt;
    if (state[SDL_SCANCODE_D]) vel.x += MOVE_SPEED * dt;

    ctx.player->set_velocity(vel);


    if (state[SDL_SCANCODE_SPACE] && ctx.player->is_on_ground()) {
        ctx.player->apply_impulse({0.0f, JUMP_FORCE});
    }

    GEngine->update(dt);
    ctx.root->process(dt);

    ctx.renderer->clear({0.2f, 0.3f, 0.3f, 1.0f});
    ctx.root->draw(ctx.renderer);
    ctx.renderer->flush();
    ctx.renderer->present();
}

int main(int argc, char* argv[]) {
    if (!GEngine->initialize(WINDOW_WIDTH, WINDOW_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return SDL_APP_FAILURE;
    }

    const auto renderer = GEngine->get_renderer();

    if (!renderer->load_font("fonts/Minecraft.ttf", "mine", 16)) {
        return SDL_APP_FAILURE;
    }

    auto sample_texture  = renderer->load_texture("sprites/Character_001.png");
    auto sample_texture2 = renderer->load_texture("sprites/Character_002.png");

    Node2D* root = new Node2D("Root");
    ctx.root = root;
    ctx.renderer = renderer;
    ctx.quit = false;

    RigidBody2D* ground = new RigidBody2D();
    ground->body_size = glm::vec2(320, 20.0f);
    ground->set_transform({glm::vec2(320 / 2, 160), glm::vec2(1.f), 0.0f});
    root->add_child("Ground", ground);

    RigidBody2D* platform1 = new RigidBody2D();
    platform1->body_size = {80, 10};
    platform1->set_transform({{120, 120}, {1.f, 1.f}, 0.0f});
    root->add_child("Platform1", platform1);

    RigidBody2D* platform2 = new RigidBody2D();
    platform2->body_size = {60, 10};
    platform2->set_transform({{220, 80}, {1.f, 1.f}, 0.0f});
    root->add_child("Platform2", platform2);

    RigidBody2D* platform3 = new RigidBody2D();
    platform3->body_size = {70, 10};
    platform3->set_transform({{80, 50}, {1.f, 1.f}, 0.0f});
    root->add_child("Platform3", platform3);

    RigidBody2D* ice_platform = new RigidBody2D();
    ice_platform->body_type = BodyType::STATIC;
    ice_platform->body_size = {100, 20};
    ice_platform->shape_type = ShapeType::RECTANGLE;
    ice_platform->color = {0.5f, 0.8f, 1.0f, 0.5f};
    ice_platform->friction = 0.05f;
    ice_platform->set_transform({{220, 140}, {1.f, 1.f}, 0.0f});
    root->add_child("IcePlatform", ice_platform);

    RigidBody2D* player = new RigidBody2D();
    player->body_type = BodyType::DYNAMIC;
    player->body_size = {16, 16};
    player->offset = glm::vec2(16);
    player->shape_type = ShapeType::CIRCLE;
    player->radius = 8.0f;
    player->restitution = 0.5f;
    player->set_transform({{50, 50}, {1.f, 1.f}, 0.0f});
    ctx.player = player;

    RigidBody2D* ice_block = new RigidBody2D();
    ice_block->body_type = BodyType::DYNAMIC;
    ice_block->body_size = {20, 20};
    ice_block->shape_type = ShapeType::RECTANGLE;
    ice_block->color = {0.5f, 0.8f, 1.0f, 0.7f};
    ice_block->set_transform({{80, 80}, {1.f, 1.f}, 0.0f});
    root->add_child("IceBlock", ice_block);

    Sprite2D* player_sprite = new Sprite2D(sample_texture2);
    player_sprite->set_region({0, 0, 32, 32}, {32, 32});
    player_sprite->set_z_index(10);
    player->add_child("Sprite", player_sprite);
    root->add_child("Player", player);

    Label* instructions = new Label("mine", "instructions");
    instructions->set_text("A/D: Move, Space: Jump");
    instructions->set_transform({{10, 0}, {1.f, 1.f}, 0.0f});
    root->add_child("Instructions", instructions);

    Label* colliding = new Label("mine", "colliding");
    colliding->set_text("Colliding with: None");
    colliding->set_transform({{10, 20}, {1.f, 1.f}, 0.0f});
    ctx.colliding = colliding;
    root->add_child("CollidingTxt", colliding);

    root->ready();

    player->on_body_entered([&](const Node2D* other) {
        if (other) {
            colliding->set_text("Colliding with: [color=#028900]%s[/color]", other->get_name().c_str());
        }
    });

    player->on_body_exited([&](const Node2D* _) {
        colliding->set_text("Colliding with: None");
    });

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(game_update, 0, true);
#else
    while (!ctx.quit) {
        game_update();
    }
#endif

    delete root;
    GEngine->shutdown();

    return 0;
}
