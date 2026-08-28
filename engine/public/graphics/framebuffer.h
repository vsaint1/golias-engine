#pragma once
#include "graphics/texture.h"

namespace golias {


    class Framebuffer {
    public:
        explicit Framebuffer(const TextureDesc& targetDesc);
        ~Framebuffer();

        void SetColorAttachment(uint32_t index, const Ref<Texture>& texture, int32_t layerOrFace = 0);

        void SetDepthAttachment(const Ref<Texture>& texture, int32_t layerOrFace = 0);

        void Bind() const;

        void Unbind() const;

        bool IsComplete() const;

    private:
        void Attach(GLenum attachment, const Ref<Texture>& texture, int32_t layerOrFace);

    private:
        GLuint mHandle = 0;
        TextureDesc mDesc;
        uint32_t mColorAttachmentCount = 0;
        bool mHasDepthAttachment       = false;
    };

} // namespace golias
