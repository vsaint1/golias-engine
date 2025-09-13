#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>

int WINDOW_WIDTH  = 1280;
int WINDOW_HEIGHT = 720;


class MainScene final : public Scene {
    RigidBody2D* player    = nullptr;
    Control* ui            = nullptr;
    Label* colliding_label = nullptr;

    const double MOVE_SPEED = 200.0f;
    const double JUMP_FORCE = 10.0f;

public:
    void on_ready() override {
        LOG_INFO("MainScene::on_ready()");

        // Load assets
        auto player_texture = GEngine->get_renderer()->load_texture("sprites/Character_002.png");


        // Ground
        auto ground             = new RigidBody2D();
        ground->collision_shape = std::make_unique<RectangleShape>(glm::vec2(320, 32));
        ground->set_transform({glm::vec2(160, 160), glm::vec2(1.f), 0.0f});
        _root->add_child("Ground", ground);

        // Player
        player                  = new RigidBody2D();
        player->collision_shape = std::make_unique<CircleShape>(8.f, BodyType::DYNAMIC);
        player->set_transform({{50, 50}, {1.f, 1.f}, 0.0f});

        auto sprite = new Sprite2D(player_texture);
        sprite->set_region({0, 0, 32, 32}, {32, 32});
        sprite->set_z_index(10);
        player->add_child("Sprite", sprite);

        _root->add_child("Player", player);

        // UI
        ui = new Control();

        auto button        = new Button("Olá, Mundo!", glm::vec2(10, 5));
        button->on_pressed = [] {
            FileAccess save("user://savefile.txt", ModeFlags::WRITE);
            if (save.store_string("hello world")) {
                LOG_INFO("Saved file to disk");
            }

            HttpRequest req("https://jsonplaceholder.typicode.com/todos/1");
            HttpClient client;
            client.request_async(req, [](const HttpResponse& resp) { LOG_INFO("HTTP Response: %s", resp.body.c_str()); });
        };

        auto button2 = new Button("Disabled", glm::vec2(300, 5), "mine");
        button2->set_disabled(true);

        colliding_label = new Label("Default", "colliding");
        colliding_label->set_text("Colliding with: None");
        colliding_label->set_transform({{10, 50}, {1, 1}, 0});

        ui->add_child("Button", button);
        ui->add_child("Button2", button2);
        ui->add_child("Label", colliding_label);

        _root->add_child("UI", ui);

        // Player collision signals
        player->on_body_entered([&](const Node2D* other) {
            if (other && colliding_label) {
                colliding_label->set_text("Colliding with: [color=#AC1C50]%s[/color]", other->get_name().c_str());
            }
        });

        player->on_body_exited([&](const Node2D*) {
            if (colliding_label) {
                colliding_label->set_text("Colliding with: None");
            }
        });
    }


    void on_update(double dt) override {
        glm::vec2 vel = player->get_velocity();

        if (GEngine->input_manager()->is_key_pressed(SDL_SCANCODE_A)) {
            vel.x -= MOVE_SPEED * dt;
        }
        if (GEngine->input_manager()->is_key_pressed(SDL_SCANCODE_D)) {
            vel.x += MOVE_SPEED * dt;
        }

        player->set_velocity(vel);

        if (GEngine->input_manager()->is_key_pressed(SDL_SCANCODE_SPACE) && player->is_on_ground()) {
            player->apply_impulse({0, JUMP_FORCE});
        }
    }
};

SDL_Event e;

void engine_core_loop() {

    while (SDL_PollEvent(&e)) {
#if defined(WITH_EDITOR)
        ImGui_ImplSDL3_ProcessEvent(&e);
#endif

        GEngine->input_manager()->process_event(e);
    }

    if (GEngine->input_manager()->is_key_pressed(SDL_SCANCODE_1)) {
        GEngine->get_system<SceneManager>()->set_scene("Test");
    }

    if (GEngine->input_manager()->is_key_pressed(SDL_SCANCODE_2)) {
        GEngine->get_system<SceneManager>()->set_scene("Main");
    }

    const double dt = GEngine->time_manager()->get_delta_time();

    GEngine->get_renderer()->clear({0.2f, 0.3f, 0.3f, 1.0f});

    GEngine->update(dt);

    GEngine->get_renderer()->flush();

    GEngine->get_renderer()->present();
}

class TestScene final : public Scene {
public:
    void on_ready() override {
        LOG_INFO("TestScene::on_ready()");
        auto label = new Label("mine", "This is a test scene!");
        label->set_transform({{100, 100}, {1, 1}, 0});
        _root->add_child("Label", label);
    }
};


// -------------------- Main --------------------
int main(int argc, char* argv[]) {
    if (!GEngine->initialize(WINDOW_WIDTH, WINDOW_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return SDL_APP_FAILURE;
    }

    const auto renderer = GEngine->get_renderer();

    auto scene_mgr = GEngine->get_system<SceneManager>();
    scene_mgr->add_scene("Main", std::make_unique<MainScene>());
    scene_mgr->add_scene("Test", std::make_unique<TestScene>());
    scene_mgr->set_scene("Test");

    if (!renderer->load_font("fonts/Minecraft.ttf", "mine", 16)) {
        return SDL_APP_FAILURE;
    }


    auto sample_texture2 = renderer->load_texture("sprites/Character_002.png");

    auto client = new ENetClient("127.0.0.1",1234);

    struct PlayerPos {
        Uint32 x;
        Uint32 y;
    };

    auto pos = PlayerPos{100,200};
    client->send(1, "Hello from client!");
    client->send(2, pos);

#if defined(SDL_PLATFORM_EMSCRIPTEN)
    emscripten_set_main_loop(engine_core_loop, 0, true);
#else
    while (GEngine->is_running) {
        client->pool();
        engine_core_loop();
    }
#endif


    delete client;
    GEngine->shutdown();

    return 0;
}
