#include "core/renderer/metal/ember_mtl.h"
#include <SDL3/SDL_main.h>


int VIRTUAL_SCREEN_WIDTH  = 1280;
int VIRTUAL_SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {


    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Metal Renderer Test",
                                          VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_METAL);

    MetalRenderer* mtl = new MetalRenderer();

    mtl->Window = window;
    mtl->Viewport[0] = 320;
    mtl->Viewport[1] = 180;
    mtl->initialize();
    
    auto sample_texture = mtl->load_texture("sprites/Character_001.png");

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
        }

     
            
            
            mtl->clear({0.2,0.3,0.3,1});
            
            mtl->draw_rect({0,20,100,50}, 0,{1,0,0,1},true,0);
            
            mtl->draw_texture(sample_texture.get(), {50.f, 20.f, 512, 256}, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {0, 0, 64, 64},
                                      0, UberShader::none());
            
            mtl->flush();
            
            mtl->present();
        


    }


    SDL_DestroyWindow(window);

    SDL_Quit();
    mtl->destroy();
    delete mtl;

    return 0;
}
