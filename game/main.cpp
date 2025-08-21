#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int VIRTUAL_SCREEN_WIDTH  = 1280;
int VIRTUAL_SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {


    if (!GEngine->initialize(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT, Backend::GL_COMPATIBILITY)) {
        return -1;
    }

    auto sample_texture = GEngine->get_renderer()->load_texture("sprites/Character_001.png");

    GEngine->get_renderer()->load_font("fonts/OpenSans.ttf",  "default",32);
    GEngine->get_renderer()->load_font("fonts/Minecraft.ttf",  "mine",16);

    while (GEngine->is_running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                GEngine->is_running = false;
            }

            const auto* pKey = SDL_GetKeyboardState(NULL);

            if (pKey[SDL_SCANCODE_E]) {
                GEngine->resize_window(960,540);
            }

            if (pKey[SDL_SCANCODE_Q]) {
                GEngine->resize_window(1280,720);
            }


        }


        GEngine->get_renderer()->clear();

        GEngine->get_renderer()->draw_line(50, 50, 100, 100, 1.0f,0, glm::vec4(0.f, 1.0f, 0.0f, 1.0f), 5);
        GEngine->get_renderer()->draw_rect({10, 10, 100, 50}, 0, {1, 1, 0, 1}, false, 0);
        GEngine->get_renderer()->draw_text("Hello [color=#FF0000]Ember[/color], [b]no bitches?[/b].", 20, 20, 0, 1.0f, {1, 1, 1, 1}, "mine", 0, UberShader::none());

        GEngine->get_renderer()->draw_texture(sample_texture.get(), {35, 35, 0, 0}, 0, {1,1,1,1}, {0,0,32,32}, 0, UberShader::none());


        GEngine->get_renderer()->flush();

        GEngine->get_renderer()->present();
    }


    GEngine->shutdown();

    return 0;
}
