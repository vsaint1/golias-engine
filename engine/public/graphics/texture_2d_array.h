#pragma once
#include "graphics/texture.h"

namespace golias {

    class Texture2DArray final : public Texture {
    public:
        explicit Texture2DArray(const TextureDesc& desc);
        ~Texture2DArray() override;

        GLuint GetHandle() const override;

        GLenum GetTarget() const override;

        const TextureDesc& GetDesc() const override;

        bool Recreate(const TextureDesc& desc) override;

    private:
        GLuint mTextureID = 0;
        TextureDesc mDesc;
    };

} // namespace golias
