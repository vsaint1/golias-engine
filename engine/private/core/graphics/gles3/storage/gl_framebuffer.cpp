#include "core/graphics/gles3/storage/gl_framebuffer.h"

#include "core/graphics/gles3/gl_common.h"
#include "core/graphics/gles3/storage/gl_texture_2d.h"
#include <spdlog/spdlog.h>
#include "core/engine.h"

namespace golias {


    OpenglFramebuffer::OpenglFramebuffer(const FramebufferSpec& specification) : spec(specification) {
        Invalidate();
    }

    OpenglFramebuffer::~OpenglFramebuffer() {
        Cleanup();
    }

    void OpenglFramebuffer::Invalidate() {
        Cleanup();

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        colorAttachments.clear();
        depthAttachment.reset();

        for (const auto& attachment : spec.attachments) {
            if (IsDepthFormat(attachment.format)) {
                depthAttachment = std::make_shared<OpenglTexture2D>(spec.width, spec.height, attachment.format);

                GLuint handle = static_cast<OpenglTexture2D*>(depthAttachment.get())->GetNativeHandle();

                GLenum glAttachment = (attachment.format == ETextureFormat::DEPTH24_STENCIL8
                                       || attachment.format == ETextureFormat::DEPTH32F_STENCIL8)
                                        ? GL_DEPTH_STENCIL_ATTACHMENT
                                        : GL_DEPTH_ATTACHMENT;

                glFramebufferTexture2D(GL_FRAMEBUFFER, glAttachment, GL_TEXTURE_2D, handle, 0);
            } else {
                auto texture = std::make_shared<OpenglTexture2D>(spec.width, spec.height, attachment.format);

                GLuint handle = static_cast<OpenglTexture2D*>(texture.get())->GetNativeHandle();

                GLenum attachmentPoint = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(colorAttachments.size());

                glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, GL_TEXTURE_2D, handle, 0);

                colorAttachments.push_back(texture);
            }
        }

        if (colorAttachments.empty()) {
            glDrawBuffers(0, nullptr);
            glReadBuffer(GL_NONE);
        } else {
            std::vector<GLenum> buffers;
            for (Uint32 i = 0; i < colorAttachments.size(); ++i) {
                buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
            }
            glDrawBuffers(buffers.size(), buffers.data());
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            spdlog::error("OpenglFramebuffer::Invalidate incomplete!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenglFramebuffer::Cleanup() {
        if (fbo) {
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
        }

        colorAttachments.clear();
        depthAttachment.reset();
    }

    void OpenglFramebuffer::Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, spec.width, spec.height);
    }

    void OpenglFramebuffer::Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        auto viewport = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice()->GetViewport();
        glViewport(0, 0, viewport.width, viewport.height);
    }

    void OpenglFramebuffer::Resize(Uint32 width, Uint32 height) {
        if (width == 0 || height == 0) {
            return;
        }
        spec.width  = width;
        spec.height = height;
        Invalidate();
    }

    bool OpenglFramebuffer::IsValid() const {
        return fbo != 0;
    }

    const FramebufferSpec& OpenglFramebuffer::GetSpecification() const {
        return spec;
    }

    std::shared_ptr<Texture2D> OpenglFramebuffer::GetColorAttachment(Uint32 index) const {
        if (index < colorAttachments.size()) {
            return colorAttachments[index];
        }
        return nullptr;
    }

    std::shared_ptr<Texture2D> OpenglFramebuffer::GetDepthAttachment() const {
        return depthAttachment;
    }

} // namespace golias
