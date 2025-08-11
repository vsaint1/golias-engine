#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int VIRTUAL_SCREEN_WIDTH  = 1280;
int VIRTUAL_SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {

    if (!GEngine->initialize(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT, RendererType::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return SDL_APP_FAILURE;
    }


    auto tex = GEngine->get_renderer()->load_texture("sprites/Character_001.png");

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        GEngine->get_renderer()->clear();

        GEngine->get_renderer()->draw_texture(tex.get(), {50.f, 400.f, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {0, 0, 64, 64},
                                             0, UberShader::shadow_only());

        GEngine->get_renderer()->flush();
        GEngine->get_renderer()->present();
    }



    GEngine->shutdown();

    return 0;
}
