#pragma once

#include "core/graphics/texture_2d.h"
#include <memory>
#include <vector>

namespace golias {

    enum class EFramebufferAttachment {
        COLOR_ATTACHMENT0 = 0,
        COLOR_ATTACHMENT1 = 1,
        COLOR_ATTACHMENT2 = 2,
        COLOR_ATTACHMENT3 = 3,
        DEPTH_ATTACHMENT,
        STENCIL_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT
    };

    enum class EFramebufferTextureFormat {
        RGBA8,
        RGB8,
        RGBA16F,
        RGB16F,
        RGBA32F,
        RGB32F,
        DEPTH24,
        DEPTH32F,
        DEPTH24_STENCIL8,
        DEPTH32F_STENCIL8
    };

    struct FramebufferAttachmentSpec {
        EFramebufferAttachment attachment;
        EFramebufferTextureFormat format;
        
        FramebufferAttachmentSpec() = default;
        FramebufferAttachmentSpec(EFramebufferAttachment att, EFramebufferTextureFormat fmt)
            : attachment(att), format(fmt) {}
    };

    struct FramebufferSpec {
        Uint32 width = 1280;
        Uint32 height = 720;
        Uint32 samples = 1; // For MSAA
        std::vector<FramebufferAttachmentSpec> attachments;
        bool swapChainTarget = false; /// Is this framebuffer the target of the swap chain (i.e., the default framebuffer)?
    };

    class Framebuffer {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        
        virtual void Resize(Uint32 width, Uint32 height) = 0;
        
        virtual Uint32 GetColorAttachmentHandle(Uint32 index = 0) const = 0;
        virtual Uint32 GetDepthAttachmentHandle() const = 0;
        
        virtual const FramebufferSpec& GetSpecification() const = 0;
        
        virtual bool IsValid() const = 0;

    protected:
        Framebuffer() = default;
    };

} // namespace golias
