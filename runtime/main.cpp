#include "core/ember_core.h"
#include <SDL3/SDL_main.h>

int WINDOW_WIDTH  = 1280;
int WINDOW_HEIGHT = 720;


const char* txt = R"(
Olá, mundo, This is some [color=#ff0000]colored[/color] text.
I need to implement harfbuzz
for proper Internationalization support!
1234567890)";


class CheckBox final : public Panel {
public:
    CheckBox(const glm::vec2& pos, const glm::vec2& size);

    void ready() override;
    void draw(Renderer* renderer) override;
    void input(const InputManager* input) override;

    void set_checked(bool checked);
    [[nodiscard]] bool is_checked() const;

    std::function<void(bool)> on_toggled;

private:
    bool _was_pressed = false;
    bool _checked     = false;
    std::weak_ptr<Texture> _checkmark_texture;
};


void CheckBox::ready() {
    _checkmark_texture = GEngine->get_renderer()->get_texture("ui/icons/icons_64.png");
    Panel::ready();
}

CheckBox::CheckBox(const glm::vec2& pos, const glm::vec2& size) {
    _panel_rect = {pos.x, pos.y, size.x, size.y};
}

void CheckBox::draw(Renderer* renderer) {

    Panel::draw(renderer);

    if (const auto tex = _checkmark_texture.lock()) {

        if (_checked) {
            float center_x = _panel_rect.x + (_panel_rect.width) / 2.0f;
            float center_y = _panel_rect.y + (_panel_rect.height) / 2.0f;

            renderer->draw_texture(tex.get(), {center_x, center_y, _panel_rect.width, _panel_rect.height}, 0, _style.checkmark_color,
                                   {512, 320, 64, 64}, _z_index + 1);
        }
    }

    std::string text = "Label";
    renderer->draw_text(text, _panel_rect.x + _panel_rect.width + 4, _panel_rect.y + (_panel_rect.height - 20) * 0.5f, 0, 1.0f,
                        _style.text_color, "Default", _z_index + 1);
}

void CheckBox::input(const InputManager* input) {
    glm::vec2 mouse = input->screen_to_world(input->get_mouse_position());
    bool inside     = input->position_in_rect(mouse, _panel_rect);

    bool pressed = input->is_action_pressed("ui_accept");

    if (inside && pressed && !_was_pressed) {
        _checked = !_checked;
        if (on_toggled) {
            on_toggled(_checked);
        }
        _was_pressed = true;
    }

    if (!pressed) {
        _was_pressed = false;
    }
}

void CheckBox::set_checked(bool checked) {
    _checked = checked;
}

bool CheckBox::is_checked() const {
    return _checked;
}

class TestScene final : public Scene {
public:
    void on_ready() override {
        LOG_INFO("TestScene::on_ready()");
        auto label = new Label("Default", txt);
        label->set_transform({{100, 100}, {1, 1}, 0});

        auto button    = new Button("Button", {100, 300}, "mine");
        auto line_edit = new LineEdit({100, 50}, {200, 32}, "Placeholder");
        auto checkbox  = new CheckBox({100, 100}, {16, 16});

        checkbox->on_toggled = [](bool checked) { LOG_INFO("Checkbox Toggled"); };

        button->on_pressed = []() { LOG_INFO("Im just a simple callback"); };

        _root->add_child("Checkbox", checkbox);
        _root->add_child("LineEdit", line_edit);
        _root->add_child("Btn", button);
        _root->add_child("Label", label);
    }

    void on_input(const SDL_Event& event) override {
        //
        // if (event.type == SDL_EVENT_MOUSE_MOTION) {
        //     LOG_INFO("Mouse moved to: (%.2f, %.2f)", event.motion.x, event.motion.y);
        // }
    }

    void on_input(const InputManager* input) override {
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

    GEngine->input_manager()->register_action(InputAction("ui_accept")
                                                  .bind_key(SDL_SCANCODE_RETURN)
                                                  .bind_key(SDL_SCANCODE_SPACE)
                                                  .bind_mouse(MouseButton::LEFT)
                                                  .bind_gamepad_button(SDL_GAMEPAD_BUTTON_SOUTH));


    const auto scene_mgr = GEngine->get_system<SceneManager>();
    scene_mgr->add_scene("Test", std::make_unique<TestScene>());
    scene_mgr->set_scene("Test");


    GEngine->run();

    GEngine->shutdown();

    return 0;
}
