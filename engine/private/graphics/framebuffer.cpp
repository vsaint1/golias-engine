#include "graphics/framebuffer.h"

namespace golias {

    Framebuffer::Framebuffer(const TextureDesc& targetDesc) : mDesc(targetDesc) {
        glGenFramebuffers(1, &mHandle);
    }

    Framebuffer::~Framebuffer() {
        if (mHandle) {
            glDeleteFramebuffers(1, &mHandle);
        }
    }

    void Framebuffer::Attach(GLenum attachment, const Ref<Texture>& texture, int32_t layerOrFace) {

        glBindFramebuffer(GL_FRAMEBUFFER, mHandle);

        if (texture->GetTarget() == GL_TEXTURE_2D_ARRAY) {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, texture->GetHandle(), 0, layerOrFace);
        } else if (texture->GetTarget() == GL_TEXTURE_CUBE_MAP) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + layerOrFace, texture->GetHandle(), 0);
        } else {
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture->GetTarget(), texture->GetHandle(), 0);
        }
    }

    void Framebuffer::SetColorAttachment(uint32_t index, const Ref<Texture>& texture, int32_t layerOrFace) {
        Attach(GL_COLOR_ATTACHMENT0 + index, texture, layerOrFace);
        mColorAttachmentCount = std::max(mColorAttachmentCount, index + 1);
    }

    void Framebuffer::SetDepthAttachment(const Ref<Texture>& texture, int32_t layerOrFace) {
        Attach(GL_DEPTH_ATTACHMENT, texture, layerOrFace);
        mHasDepthAttachment = true;
    }

    void Framebuffer::Bind() const {

        glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
        glViewport(0, 0, mDesc.Width, mDesc.Height);

        if (mColorAttachmentCount == 0 && mHasDepthAttachment) {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        } else {
            std::vector<GLenum> buffers(mColorAttachmentCount);
            for (uint32_t i = 0; i < mColorAttachmentCount; ++i) {
                buffers[i] = GL_COLOR_ATTACHMENT0 + i;
            }
            glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
        }
    }

    void Framebuffer::Unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    bool Framebuffer::IsComplete() const {
        glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
        return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }
} // namespace golias
