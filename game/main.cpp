#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int SCREEN_WIDTH  = 1366;
int SCREEN_HEIGHT = 768;


int main(int argc, char* argv[]) {

    if (!GEngine->initialize("Example - new API", SCREEN_WIDTH, SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }


    if (!GEngine->get_renderer()->load_font("fonts/Minecraft.ttf", "mine", 48)) {
        LOG_ERROR("failed to load mine font");
        return SDL_APP_FAILURE;

    }

    if (!GEngine->get_renderer()->load_font("fonts/Arial.otf", "arial", 48)) {
        LOG_ERROR("failed to load arial font");
        return SDL_APP_FAILURE;
    }

    Texture& sample_texture  = GEngine->get_renderer()->load_texture("sprites/Character_001.png");
    Texture& sample_texture2 = GEngine->get_renderer()->load_texture("sprites/Character_002.png");

    bool quit                = false;
    SDL_Event e;
    float angle = 0.0f;

    Node2D* root = new Node2D("Root");

    Node2D* player = new Node2D();
    player->set_z_index(5);
    player->set_transform({glm::vec2(150.f, 100.f), glm::vec2(1.f), 0.0f});

    Sprite2D* player_sprite = new Sprite2D(sample_texture);
    player_sprite->set_region({0, 0, 32, 32}, glm::vec2(128));

    player->add_child("Image", player_sprite);

    root->add_child("player",player);

    root->ready();
    root->print_tree();

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }
        angle += 0.01f;
        if (angle > 2 * M_PI) {
            angle -= 2 * M_PI;
        }


        GEngine->get_renderer()->clear(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));

        root->draw(GEngine->get_renderer());

        // GEngine->get_renderer()->draw_rect({100, 100, 200, 150}, angle, glm::vec4(1.0f, 0.5f, 0.2f, 1.0f), true, 1);
        // GEngine->get_renderer()->draw_circle(400, 300, 0, 80, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f), false, 32, 2);
        // GEngine->get_renderer()->draw_triangle(500, 100, 600, 200, 450, 200,0, glm::vec4(0.8f, 0.2f, 0.8f, 1.0f), false, 3);
        // GEngine->get_renderer()->draw_line(50, 50, 750, 550, 3.0f,0, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 5);
        // std::vector<glm::vec2> polygon_points = {{200, 400}, {250, 380}, {300, 420}, {280, 480}, {220, 490}};
        // GEngine->get_renderer()->draw_polygon(polygon_points, 0, glm::vec4(0.5f, 0.5f, 1.0f, 1.0f), true, 1);
        //
        //
        // for (int i = 0; i < 100; ++i) {
        //     GEngine->get_renderer()->draw_texture(sample_texture, {i * 50.f, i * 50.f, 512, 256}, angle, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        //                                           {0, 0, 64, 64}, 0);
        // }
        //
        // GEngine->get_renderer()->draw_texture(sample_texture2, {400, 200, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {}, 5);
        // GEngine->get_renderer()->draw_text("Hola amigo, como estas?", 100, 60, 0, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "mine", 10);
        // GEngine->get_renderer()->draw_text("Ola mundo", 100, 250, 0, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "arial", 10);
        // GEngine->get_renderer()->draw_text("Hello world", 100, 350, 0, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "mine", 10);
        GEngine->get_renderer()->draw_text("Hello [color=#FF0000]World[/color]", 100, 500, 0, 1.f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "mine", 10);


        GEngine->get_renderer()->flush();
    }

    delete root;

    GEngine->shutdown();

    return 0;
}
