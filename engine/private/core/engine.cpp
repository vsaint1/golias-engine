#include "core/engine.h"

// 💀

Core core;

ma_engine engine;


void Core::Resize(int w, int h) const {
    SDL_assert(w > 0 && h > 0);
    
    core.Window.width= w;
    core.Window.height = h;


}