#pragma once

#include "core/renderer/base_struct.h"


/*!

    @file sdl_struct.h
    @brief SDL specific structures for Font and Texture.
   
    This file contains the definitions of SDLFont and SDLTexture classes, which are concrete implementations of the Font and Texture interfaces using the SDL library.

    @version 0.0.1

*/
class SDLTexture : public Texture {
public:
    SDLTexture(SDL_Texture* tex) : _texture(tex) {
    }

    ~SDLTexture() override;

    SDL_Texture* get_texture() const;

private:
    SDL_Texture* _texture = nullptr;
};