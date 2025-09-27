#pragma once

#include "core/renderer/base_struct.h"


class SDLFont : public Font {

public:
    SDLFont(TTF_Font* f) : _font(f) {
    }

    ~SDLFont() override;

    glm::vec2 get_size(const std::string& text) override;

    TTF_Font* get_font() const;

private:
    TTF_Font* _font = nullptr;
};


class SDLTexture : public Texture {
public:
    SDLTexture(SDL_Texture* tex) : _texture(tex) {
    }

    ~SDLTexture() override;

    SDL_Texture* get_texture() const;

private:
    SDL_Texture* _texture = nullptr;
};