#pragma once
#include "graphics/texture.h"

namespace golias {

    class TextureCube final : public Texture {
    public:
        explicit TextureCube(const TextureDesc& desc);
        ~TextureCube() override;

        GLuint GetHandle() const override;

        GLenum GetTarget() const override;

        const TextureDesc& GetDesc() const override;

        bool Recreate(const TextureDesc& desc) override;

    private:
        GLuint mTextureID = 0;
        TextureDesc mDesc;
    };
} // namespace golias
