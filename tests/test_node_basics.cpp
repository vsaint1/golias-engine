#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>

int WINDOW_WIDTH  = 1280;
int WINDOW_HEIGHT = 720;

int main(int argc, char* argv[]) {

    if (!GEngine->initialize(WINDOW_WIDTH, WINDOW_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return SDL_APP_FAILURE;
    }

    auto renderer = GEngine->get_renderer();


    GEngine->set_vsync(true);

    if (!renderer->load_font("fonts/Minecraft.ttf", "mine", 16)) {
        LOG_ERROR("failed to load mine font");
        return SDL_APP_FAILURE;
    }

    auto sample_texture  = renderer->load_texture("sprites/Character_001.png");
    auto sample_texture2 = renderer->load_texture("sprites/Character_002.png");

    bool quit = false;
    SDL_Event e;
    float angle = 0.0f;

    Node2D* root = new Node2D("Root");

    Node2D* player = new Node2D();
    player->set_z_index(5);
    player->set_transform({glm::vec2(150.f, 100.f), glm::vec2(1.f), 0.0f});

    Label* name = new Label("mine", "golias_bento");
    name->set_text("Hello [color=#FF0000]World[/color], [b]no bold?[/b].\nPlayer Health  [color=#028900]%d[/color] %s", 100, "robson");
    name->set_shadow(true);
    name->set_transform({glm::vec2(100.f, 110.f), glm::vec2(1.f), 0.0f});

    Sprite2D* player_sprite = new Sprite2D(sample_texture2);
    player_sprite->set_region({0, 0, 32, 32}, glm::vec2(32));
    player_sprite->set_z_index(10);

    player->add_child("Image", player_sprite);

    root->add_child("Player", player);
    root->add_child("Text",name);

    root->ready();
    root->print_tree();

    bool is_filled = true;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }

            const auto& pKey = SDL_GetKeyboardState(nullptr);
            if (pKey[SDL_SCANCODE_SPACE]) {
                is_filled = !is_filled;
            }
        }

        angle += 0.01f;
        if (angle > 2 * M_PI) angle -= 2 * M_PI;

        renderer->clear(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));

        root->draw(renderer);

        renderer->draw_rect({100, 50, 50, 30}, 0.0f, glm::vec4(1.0f,0.5f,0.2f,1.0f), is_filled,1);
        renderer->draw_circle(160, 90, 0, 20, glm::vec4(0.2f,0.8f,0.2f,1.0f), is_filled, 32, 2);
        renderer->draw_line(10, 10, 300, 170, 1.0f, 0, glm::vec4(1.0f,1.0f,0.0f,1.0f), 2);
        renderer->draw_texture(sample_texture.get(), {50,50,32,32}, 0, glm::vec4(1.0f), {192,0,32,32}, 0, UberShader::outline_only());

        renderer->draw_triangle(0, 30, 40, 30, 20, 0, 0.0f, glm::vec4(0.8f, 0.1f, 0.5f, 1.0f), is_filled, 5);

        renderer->draw_texture(sample_texture2.get(), {50,100,32,32}, 0, glm::vec4(1.0f), {0,0,32,32}, 0, UberShader::shadow_only());

        renderer->flush();
        renderer->present();
    }

    delete root;
    GEngine->shutdown();

    return 0;
}
