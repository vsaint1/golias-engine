#pragma once

#include "core/graphics/framebuffer.h"

namespace golias {

    class GLFramebuffer : public Framebuffer {
    public:
        GLFramebuffer(const FramebufferSpec& spec);
        ~GLFramebuffer() override;

        void Bind() override;
        void Unbind() override;
        
        void Resize(Uint32 width, Uint32 height) override;
        
        Uint32 GetColorAttachmentHandle(Uint32 index = 0) const override;
        Uint32 GetDepthAttachmentHandle() const override;
        
        const FramebufferSpec& GetSpecification() const override { return spec; }
        
        bool IsValid() const override { return fbo != 0; }

    private:
        void Invalidate();
        void Cleanup();

        FramebufferSpec spec;
        Uint32 fbo = 0;
        
        std::vector<Uint32> colorAttachments;
        Uint32 depthAttachment = 0;
    };

} // namespace golias
