#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int VIRTUAL_SCREEN_WIDTH  = 1280;
int VIRTUAL_SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {


    if (!GEngine->initialize(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT, Backend::OPENGL)) {
        return -1;
    }

    auto sample_texture = GEngine->get_renderer()->load_texture("sprites/Character_001.png");


    while (GEngine->is_running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                GEngine->is_running = false;
            }
        }


        GEngine->get_renderer()->clear();

        GEngine->get_renderer()->draw_rect({10, 10, 100, 50}, 0, {1, 0, 0, 1}, false, 0);

        GEngine->get_renderer()->draw_texture(sample_texture.get(), {0.f, 0.f, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                              {0, 0, 32, 32}, 0, UberShader::none());

        GEngine->get_renderer()->flush();

        GEngine->get_renderer()->present();
    }


    GEngine->shutdown();

    return 0;
}
