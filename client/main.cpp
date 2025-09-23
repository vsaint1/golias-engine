#include "core/engine.h"
#include <SDL3/SDL_main.h>

#define WINDOW_W 640
#define WINDOW_H 480

int main(int argc, char* argv[]) {


    if (!GEngine->initialize(WINDOW_W, WINDOW_H, "Ember Engine - Window")) {
        return SDL_APP_FAILURE;
    }


    GEngine->run();


    return SDL_APP_CONTINUE;
}
