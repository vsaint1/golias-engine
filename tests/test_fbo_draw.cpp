#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int VIRTUAL_SCREEN_WIDTH  = 1280;
int VIRTUAL_SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {

    if (!GEngine->initialize("Example - with FBO", VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT, Backend::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }

    GEngine->get_renderer()->resize_viewport(320,180);

    GEngine->set_vsync(true);

    if (!GEngine->get_renderer()->load_font("fonts/Minecraft.ttf", "mine", 48)) {
        LOG_ERROR("failed to load mine font");
        return SDL_APP_FAILURE;
    }

    if (!GEngine->get_renderer()->load_font("fonts/Arial.otf", "arial", 48)) {
        LOG_ERROR("failed to load arial font");
        return SDL_APP_FAILURE;
    }

    auto sample_texture  = GEngine->get_renderer()->load_texture("sprites/Character_001.png");
    auto sample_texture2 = GEngine->get_renderer()->load_texture("sprites/Character_002.png");

    bool quit = false;
    SDL_Event e;
    float angle = 0.0f;

    // TODO: instead of using raw pointers, we must `track` the nodes and delete them automatically ( scene manager? )
    Node2D* root = new Node2D("Root");

    Node2D* player = new Node2D();
    player->set_z_index(5);
    player->set_transform({glm::vec2(150.f, 100.f), glm::vec2(1.f), 0.0f});

    Label* name = new Label("mine", "golias_bento", 64);
    name->set_text("Hello [color=#FF0000]World[/color], [b]no bold?[/b].\nPlayer Health  [color=#028900]%d[/color] %s", 100, "robson");
    name->set_shadow(true);

    Sprite2D* player_sprite = new Sprite2D(sample_texture2);
    player_sprite->set_region({0, 0, 32, 32}, glm::vec2(128));
    player_sprite->set_z_index(10);


    player->add_child("Image", player_sprite);
    player->add_child("Name", name);

    root->add_child("Player", player);

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
        if (angle > 2 * M_PI) {
            angle -= 2 * M_PI;
        }


        GEngine->get_renderer()->clear(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));

        root->draw(GEngine->get_renderer());

        GEngine->get_renderer()->draw_rect({100, 100, 200, 150}, 0.0f, glm::vec4(1.0f, 0.5f, 0.2f, 1.0f), is_filled, 1);
        GEngine->get_renderer()->draw_circle(400, 300, 0, 80, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f), is_filled, 128, 2);
        GEngine->get_renderer()->draw_triangle(500, 100, 600, 200, 450, 200,0, glm::vec4(0.8f, 0.2f, 0.8f, 1.0f), is_filled, 3);
        GEngine->get_renderer()->draw_line(50, 50, 750, 550, 3.0f,0, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 5);

        const std::vector<glm::vec2> polygon_points = {{200, 400}, {250, 380}, {300, 420}, {280, 480}, {220, 490}};
        GEngine->get_renderer()->draw_polygon(polygon_points, 0, glm::vec4(0.5f, 0.5f, 1.0f, 1.0f), is_filled, 1);

        // for (int i = 0; i < 100; ++i) {
        //     GEngine->get_renderer()->draw_texture(sample_texture, {i * 50.f, i * 50.f, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        //                                           {0, 0, 64, 64}, 0);
        // }

        GEngine->get_renderer()->draw_texture(sample_texture.get(), {50.f, 400.f, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {0, 0, 64, 64},
                                              0, UberShader::shadow_only());
        // GEngine->get_renderer()->draw_texture(sample_texture2, {400, 200, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {}, 5,UberShader::shadow_only());

        GEngine->get_renderer()->flush();

        GEngine->get_renderer()->present();
    }

    delete root;

    GEngine->shutdown();

    return 0;
}
