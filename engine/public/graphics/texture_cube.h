#pragma once
#include "graphics/texture.h"

namespace golias {

    class TextureCube final : public Texture {
    public:
        explicit TextureCube(const TextureDesc& desc);
        ~TextureCube() override;

        static Ref<TextureCube> Load(CString path);
        static Ref<TextureCube>
            CreateProcedural(const glm::vec3& skyTint, const glm::vec3& groundColor, const glm::vec3& sunDirection, float sunBrightness);

        GLuint GetHandle() const override;

        GLenum GetTarget() const override;

        const TextureDesc& GetDesc() const override;

        bool Recreate(const TextureDesc& desc) override;

    private:
        bool UploadFace(uint32_t face, uint32_t width, uint32_t height, int channels, const unsigned char* data);
        bool UploadFaceFloat(uint32_t face, uint32_t width, uint32_t height, const float* data);

    private:
        GLuint mTextureID = 0;
        TextureDesc mDesc;
    };
} // namespace golias
