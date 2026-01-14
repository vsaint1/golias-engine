#pragma once

#include "core/graphics/texture_2d.h"
#include <memory>
#include <vector>

namespace golias {

   

    struct FramebufferAttachmentSpec {
        EFramebufferAttachment attachment;
        EFramebufferTextureFormat format;

        FramebufferAttachmentSpec() = default;
        FramebufferAttachmentSpec(EFramebufferAttachment att, EFramebufferTextureFormat fmt) : attachment(att), format(fmt) {
        }
    };

    struct FramebufferSpec {
        Uint32 width   = 1280;
        Uint32 height  = 720;
        Uint32 samples = 1; // For MSAA
        std::vector<FramebufferAttachmentSpec> attachments;
        bool swapChainTarget = false; /// Is this framebuffer the target of the swap chain (i.e., the default framebuffer)?
    };

    class Framebuffer {
    public:
        virtual ~Framebuffer() = default;

        virtual void Bind()   = 0;
        virtual void Unbind() = 0;

        virtual void Resize(Uint32 width, Uint32 height) = 0;

        virtual std::shared_ptr<Texture2D> GetColorAttachment(Uint32 index) const = 0;
        virtual std::shared_ptr<Texture2D> GetDepthAttachment() const             = 0;

        virtual const FramebufferSpec& GetSpecification() const = 0;

        virtual bool IsValid() const = 0;

    protected:
        Framebuffer() = default;
        std::vector<std::shared_ptr<Texture2D>> colorAttachments;
        std::shared_ptr<Texture2D> depthAttachment;
    };

} // namespace golias
