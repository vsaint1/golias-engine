#include "core/renderer/sdl/sdl_struct.h"




// TEXTURE IMPLEMENTATION

SDLTexture::~SDLTexture() {
    if (_texture) {
        SDL_DestroyTexture(_texture);
    }
}

SDL_Texture* SDLTexture::get_texture() const {
    return _texture;
}
