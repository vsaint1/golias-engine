#include "core/graphics/gles3/storage/gl_framebuffer.h"
#include "core/graphics/gles3/gl_common.h"
#include <spdlog/spdlog.h>

namespace golias {

    static GLenum FramebufferTextureFormatToGL(EFramebufferTextureFormat format) {
        switch (format) {
            case EFramebufferTextureFormat::RGBA8:       return GL_RGBA8;
            case EFramebufferTextureFormat::RGB8:        return GL_RGB8;
            case EFramebufferTextureFormat::RGBA16F:     return GL_RGBA16F;
            case EFramebufferTextureFormat::RGB16F:      return GL_RGB16F;
            case EFramebufferTextureFormat::RGBA32F:     return GL_RGBA32F;
            case EFramebufferTextureFormat::RGB32F:      return GL_RGB32F;
            case EFramebufferTextureFormat::DEPTH24:     return GL_DEPTH_COMPONENT24;
            case EFramebufferTextureFormat::DEPTH32F:    return GL_DEPTH_COMPONENT32F;
            case EFramebufferTextureFormat::DEPTH24_STENCIL8:   return GL_DEPTH24_STENCIL8;
            case EFramebufferTextureFormat::DEPTH32F_STENCIL8:  return GL_DEPTH32F_STENCIL8;
        }
        return GL_RGBA8;
    }

    static bool IsDepthFormat(EFramebufferTextureFormat format) {
        switch (format) {
            case EFramebufferTextureFormat::DEPTH24:
            case EFramebufferTextureFormat::DEPTH32F:
            case EFramebufferTextureFormat::DEPTH24_STENCIL8:
            case EFramebufferTextureFormat::DEPTH32F_STENCIL8:
                return true;
            default:
                return false;
        }
    }

    GLFramebuffer::GLFramebuffer(const FramebufferSpec& specification) : spec(specification) {
        spdlog::info("GLFramebuffer::GLFramebuffer Creating framebuffer of size {}x{}", spec.width, spec.height);
        Invalidate();
    }

    GLFramebuffer::~GLFramebuffer() {
        Cleanup();
    }

    void GLFramebuffer::Invalidate() {
        if (fbo) {
            Cleanup();
        }

        spdlog::info("GLFramebuffer::Invalidate Initializing framebuffer of size {}x{}", spec.width, spec.height);

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        if (!spec.attachments.empty()) {
            colorAttachments.resize(0);
            depthAttachment = 0;

            for (const auto& attachmentSpec : spec.attachments) {
                if (IsDepthFormat(attachmentSpec.format)) {
                    glGenTextures(1, &depthAttachment);
                    glBindTexture(GL_TEXTURE_2D, depthAttachment);

                    GLenum internalFormat = FramebufferTextureFormatToGL(attachmentSpec.format);
                    GLenum format = GL_DEPTH_COMPONENT;
                    GLenum type = GL_FLOAT;

                    if (attachmentSpec.format == EFramebufferTextureFormat::DEPTH24_STENCIL8 ||
                        attachmentSpec.format == EFramebufferTextureFormat::DEPTH32F_STENCIL8) {
                        format = GL_DEPTH_STENCIL;
                        type = (attachmentSpec.format == EFramebufferTextureFormat::DEPTH24_STENCIL8) 
                               ? GL_UNSIGNED_INT_24_8 : GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
                    }

                    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, spec.width, spec.height, 0, format, type, nullptr);
                    
                    
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                    
                    
                    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

                    GLenum attachment = (format == GL_DEPTH_STENCIL) ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
                    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, depthAttachment, 0);

                } else {
                    GLuint colorTexture;
                    glGenTextures(1, &colorTexture);
                    glBindTexture(GL_TEXTURE_2D, colorTexture);

                    GLenum internalFormat = FramebufferTextureFormatToGL(attachmentSpec.format);
                    GLenum format = GL_RGBA;
                    GLenum type = GL_UNSIGNED_BYTE;

                    if (attachmentSpec.format == EFramebufferTextureFormat::RGB8 ||
                        attachmentSpec.format == EFramebufferTextureFormat::RGB16F ||
                        attachmentSpec.format == EFramebufferTextureFormat::RGB32F) {
                        format = GL_RGB;
                    }

                    if (attachmentSpec.format == EFramebufferTextureFormat::RGBA16F ||
                        attachmentSpec.format == EFramebufferTextureFormat::RGB16F ||
                        attachmentSpec.format == EFramebufferTextureFormat::RGBA32F ||
                        attachmentSpec.format == EFramebufferTextureFormat::RGB32F) {
                        type = GL_FLOAT;
                    }

                    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, spec.width, spec.height, 0, format, type, nullptr);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                    GLenum attachment = GL_COLOR_ATTACHMENT0 + colorAttachments.size();
                    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, colorTexture, 0);
                    colorAttachments.push_back(colorTexture);
                }
            }

            if (colorAttachments.empty()) {
                glDrawBuffers(GL_NONE,nullptr);
                glReadBuffer(GL_NONE);
            } else {
                std::vector<GLenum> drawBuffers;
              
                for (size_t i = 0; i < colorAttachments.size(); i++) {
                    drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
                }

                glDrawBuffers(drawBuffers.size(), drawBuffers.data());
            }
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            spdlog::error("GLFramebuffer::Invalidate Framebuffer is not complete!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GLFramebuffer::Cleanup() {
        if (fbo) {
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
        }

        for (auto texture : colorAttachments) {
            glDeleteTextures(1, &texture);
        }
        colorAttachments.clear();

        if (depthAttachment) {
            glDeleteTextures(1, &depthAttachment);
            depthAttachment = 0;
        }
    }

    void GLFramebuffer::Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, spec.width, spec.height);
    }

    void GLFramebuffer::Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GLFramebuffer::Resize(Uint32 width, Uint32 height) {
        if (width == 0 || height == 0) {
            spdlog::warn("GLFramebuffer::Resize Attempted to resize framebuffer to invalid size: {}x{}", width, height);
            return;
        }

        spec.width = width;
        spec.height = height;
        Invalidate();
    }

    Uint32 GLFramebuffer::GetColorAttachmentHandle(Uint32 index) const {
        if (index < colorAttachments.size()) {
            return colorAttachments[index];
        }

        return 0;
    }

    Uint32 GLFramebuffer::GetDepthAttachmentHandle() const {
        return depthAttachment;
    }

} // namespace golias
