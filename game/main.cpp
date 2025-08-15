#include "core/renderer/opengl/ember_gl.h"
#include <SDL3/SDL_main.h>


int VIRTUAL_SCREEN_WIDTH  = 1280;
int VIRTUAL_SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {


    if (!GEngine->initialize(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT, Backend::OPENGL, SDL_WINDOW_RESIZABLE)) {
        return -1;
    }

    auto sample_texture = GEngine->get_renderer()->load_texture("sprites/Character_001.png");

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
        }


            GEngine->get_renderer()->clear({0.2,0.3,0.3,1});

            GEngine->get_renderer()->draw_rect({0,20,100,50}, 0,{1,0,0,1},false,0);

            GEngine->get_renderer()->draw_texture(sample_texture.get(), {50.f, 20.f, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {0, 0, 64, 64},
                                      0, UberShader::none());

            GEngine->get_renderer()->flush();

            GEngine->get_renderer()->present();



    }


    GEngine->shutdown();

    return 0;
}
