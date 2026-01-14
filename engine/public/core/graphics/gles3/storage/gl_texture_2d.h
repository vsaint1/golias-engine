#pragma once

#include "core/graphics/texture_2d.h"
#include <stb_image.h>

namespace golias {


    class OpenglTexture2D : public Texture2D {
    public:
        OpenglTexture2D(const std::string_view pFilePath);
        OpenglTexture2D(int w, int h, ETextureFormat fmt, Uint8* data);
        OpenglTexture2D(Uint32 width, Uint32 height, EFramebufferTextureFormat format);
        ~OpenglTexture2D() override;

    private:
        void CreateInternal(Uint8* data);
    };

} // namespace golias
