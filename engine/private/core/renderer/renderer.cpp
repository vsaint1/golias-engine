#include "core/renderer/renderer.h"


void Renderer::set_default_fonts(const std::string& text_font, const std::string& emoji_font) {
    _default_font_name = text_font;
    _emoji_font_name   = emoji_font;

    LOG_DEBUG("Default Fonts Alias: Text: %s | Emoji: %s", text_font.c_str(), emoji_font.c_str());
}


std::string Renderer::vformat(const char* fmt, va_list args) {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    return std::string(buf);
}

std::shared_ptr<Texture> Renderer::load_texture(const std::string& name, const std::string& path, const aiTexture* ai_embedded_tex) {

    SDL_Surface* surf = nullptr;

    if (ai_embedded_tex) {

        LOG_INFO("Loading embedded texture: %s (%ux%u)", name.c_str(), ai_embedded_tex->mWidth, ai_embedded_tex->mHeight);


        // (mHeight == 0 means compressed)
        if (ai_embedded_tex->mHeight == 0) {
            // Compressed texture (e.g., PNG, JPEG)
            SDL_IOStream* rw = SDL_IOFromConstMem(ai_embedded_tex->pcData, ai_embedded_tex->mWidth);
            if (!rw) {
                LOG_ERROR("Failed to create SDL_IOStream from embedded texture data");
                return nullptr;
            }

            surf = IMG_Load_IO(rw, true);
            if (!surf) {
                LOG_ERROR("Failed to load compressed embedded texture: %s", SDL_GetError());
                return nullptr;
            }
        } else {
            // Uncompressed ARGB8888 texture
            surf = SDL_CreateSurface(ai_embedded_tex->mWidth, ai_embedded_tex->mHeight, SDL_PIXELFORMAT_RGBA32);
            if (!surf) {
                LOG_ERROR("Failed to create surface for embedded texture: %s", SDL_GetError());
                return nullptr;
            }

            aiTexel* texels = ai_embedded_tex->pcData;
            Uint32* pixels  = (Uint32*) surf->pixels;

            for (unsigned int i = 0; i < ai_embedded_tex->mWidth * ai_embedded_tex->mHeight; i++) {
                // ARGB to RGBA
                Uint8 r   = texels[i].r;
                Uint8 g   = texels[i].g;
                Uint8 b   = texels[i].b;
                Uint8 a   = texels[i].a;
                pixels[i] = SDL_MapSurfaceRGBA(surf, r, g, b, a);
            }
        }
    } else {


        LOG_INFO("Loading texture: %s", path.c_str());

        FileAccess file_access(path, ModeFlags::READ);

        if (!file_access.is_open()) {
            LOG_ERROR("Failed to open texture file: %s", path.c_str());
            return nullptr;
        }

        surf = IMG_Load_IO(file_access.get_handle(), false);
        // SDL_Surface* surf = IMG_Load(path.c_str());

        if (!surf) {
            LOG_ERROR("Failed to load texture: %s, Error: %s", path.c_str(), SDL_GetError());
            return nullptr;
        }
    }

    auto texture = std::make_shared<Texture>();

    surf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);

    if (!surf) {
        LOG_ERROR("Failed to convert texture to RGBA32: %s", SDL_GetError());
        SDL_DestroySurface(surf);
        return nullptr;
    }

    texture->width  = surf->w;
    texture->height = surf->h;
    texture->path   = path;
    texture->surface = surf;

    LOG_INFO("Texture Info: Size %dx%d | Path: %s | Embedded: %s", texture->width, texture->height, texture->path.c_str(), ai_embedded_tex != nullptr ? "Yes" : "No");
    return texture;
}