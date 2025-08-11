#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int VIRTUAL_SCREEN_WIDTH  = 1280;
int VIRTUAL_SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {

    if (!GEngine->initialize(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT, Backend::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }


    auto tex = GEngine->get_renderer()->load_texture("sprites/Character_001.png");

    GEngine->get_renderer()->load_font("fonts/Minecraft.ttf", "mine", 32);

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        GEngine->get_renderer()->clear();

        // GEngine->get_renderer()->draw_texture(tex.get(), {0.f, 10.f, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {0, 0, 64, 64}, 0,
        //                                       UberShader::shadow_only());

        GEngine->get_renderer()->draw_text("Hello [color=#FF0000]World[/color]", 20, 10, 0, 1.f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "mine",
                                           0, UberShader::outline_only());

        GEngine->get_renderer()->flush();
        GEngine->get_renderer()->present();
    }


    GEngine->shutdown();

    return 0;
}
