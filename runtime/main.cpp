#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>

int WINDOW_WIDTH  = 1280;
int WINDOW_HEIGHT = 720;

struct GameContext {
    Node2D* root;
    RigidBody2D* player;
    Renderer* renderer;
    Control* ui_node;
};

GameContext ctx;
SDL_Event e;

const float MOVE_SPEED = 200.0f; // px/s
const float JUMP_FORCE = 10.0f; // px impulse


void game_update() {
    const double dt = GEngine->time_manager()->get_delta_time();

    while (SDL_PollEvent(&e)) {
#if defined(WITH_EDITOR)
        ImGui_ImplSDL3_ProcessEvent(&e);
#endif
        if (e.type == SDL_EVENT_QUIT) {
            GEngine->is_running = false;
        }
        GEngine->input_manager()->process_event(e);
    }


    glm::vec2 vel = ctx.player->get_velocity();

    if (GEngine->input_manager()->is_key_pressed(SDL_SCANCODE_A)) {
        vel.x -= MOVE_SPEED * dt;
    }

    if (GEngine->input_manager()->is_key_pressed(SDL_SCANCODE_D)) {
        vel.x += MOVE_SPEED * dt;
    }

    ctx.player->set_velocity(vel);

    if (GEngine->input_manager()->is_key_pressed(SDL_SCANCODE_SPACE) && ctx.player->is_on_ground()) {
        ctx.player->apply_impulse({0.0f, JUMP_FORCE});
    }

    GEngine->update();
    ctx.root->ready();
    ctx.root->process(dt);

    ctx.renderer->clear({0.2f, 0.3f, 0.3f, 1.0f});
    ctx.root->input(GEngine->input_manager());
    ctx.root->draw(ctx.renderer);
    ctx.renderer->flush();

    draw_editor(ctx.root);

    ctx.renderer->present();

    // GEngine->time_manager()->limit_frame_rate();
}


// -------------------- Main --------------------
int main(int argc, char* argv[]) {
    if (!GEngine->initialize(WINDOW_WIDTH, WINDOW_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return SDL_APP_FAILURE;
    }

    const auto renderer = GEngine->get_renderer();
    ctx.renderer        = renderer;

    GEngine->time_manager()->print_debug_info();

    if (!renderer->load_font("fonts/Minecraft.ttf", "mine", 16)) {
        return SDL_APP_FAILURE;
    }

    // ---- Scene setup ----
    auto sample_texture2 = renderer->load_texture("sprites/Character_002.png");

    Node2D* root = new Node2D("Root");
    ctx.root     = root;


    RigidBody2D* ground     = new RigidBody2D();
    ground->collision_shape = std::make_unique<RectangleShape>(glm::vec2(320, 32));
    ground->set_transform({glm::vec2(160, 160), glm::vec2(1.f), 0.0f});

    RigidBody2D* player     = new RigidBody2D();
    player->collision_shape = std::make_unique<CircleShape>(8.f, BodyType::DYNAMIC);
    player->set_transform({{50, 50}, {1.f, 1.f}, 0.0f});

    ctx.player = player;

    Sprite2D* player_sprite = new Sprite2D(sample_texture2);
    player_sprite->set_region({0, 0, 32, 32}, {32, 32});
    player_sprite->set_z_index(10);
    player->add_child("Sprite", player_sprite);

    Control* ui_control = new Control();

    Button* test_button         = new Button("Hello world", glm::vec2(10, 5), glm::vec2(90, 30));
    test_button->on_hover_enter = [] { LOG_INFO("im entering"); };
    test_button->on_pressed     = [] { LOG_INFO("clicked"); };

    Label* colliding = new Label("mine", "colliding");

    colliding->set_text("Colliding with: None");
    colliding->set_transform({{10, 50}, {1.f, 1.f}, 0.0f});

    ui_control->add_child("Button", test_button);
    ui_control->add_child("Label", colliding);

    ctx.ui_node = ui_control;


    root->add_child("UI", ui_control);
    root->add_child("Player", player);
    root->add_child("Ground", ground);

    player->on_body_entered([&](const Node2D* other) {
        if (other) {
            if (colliding) {
                colliding->set_text("Colliding with: %s", other->get_name().c_str());
            }
        }
    });

    player->on_body_exited([&](const Node2D* _) {
        if (colliding) {
            colliding->set_text("Colliding with: None");
        }
    });

    // ---- Main loop ----
    while (GEngine->is_running) {
        game_update();
    }

    // ---- Cleanup ----
    delete root;
    GEngine->shutdown();

    return 0;
}
