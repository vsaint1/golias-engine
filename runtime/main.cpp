#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>

int WINDOW_WIDTH  = 1280;
int WINDOW_HEIGHT = 720;

const char* text = R"(
Olá, mundo, This is some [color=#ff0000]colored[/color] text.
I need to implement harfbuzz
for proper Internationalization support!
1234567890)";

class TestScene final : public Scene {
public:
    void on_ready() override {
        LOG_INFO("TestScene::on_ready()");
        auto label = new Label("Default", text);
        label->set_transform({{100, 100}, {1, 1}, 0});
        _root->add_child("Label", label);
    }

    void on_input(InputManager* input) override {
        if (input->is_key_pressed(SDL_SCANCODE_2)) {
            GEngine->get_system<SceneManager>()->set_scene("Main");
        }
    }
};


int main(int argc, char* argv[]) {
    if (!GEngine->initialize(WINDOW_WIDTH, WINDOW_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return SDL_APP_FAILURE;
    }

    const auto renderer = GEngine->get_renderer();

    if (!renderer->load_font("fonts/Minecraft.ttf", "mine", 16)) {
        return SDL_APP_FAILURE;
    }

    const auto scene_mgr = GEngine->get_system<SceneManager>();
    scene_mgr->add_scene("Test", std::make_unique<TestScene>());
    scene_mgr->set_scene("Test");


    GEngine->run();

    GEngine->shutdown();

    return 0;
}
