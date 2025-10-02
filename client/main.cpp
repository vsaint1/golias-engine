#include "core/engine.h"
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
    const int WINDOW_W = 1280, WINDOW_H = 720;


    if (!GEngine->initialize(WINDOW_W, WINDOW_H)) {
        LOG_INFO("Failed to initialize engine");
        return -1;
    }

    GEngine->run();

    return 0;
}
