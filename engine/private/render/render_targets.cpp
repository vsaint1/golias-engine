#include "render/render_targets.h"

#include "core/engine.h"
#include "render/csm.h"

namespace golias {

    bool RenderTargets::EnsureHdrLdr(const Viewport& viewport) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (mHdrFramebuffer && viewport.Width == mHdrViewport.Width && viewport.Height == mHdrViewport.Height) {
            return true;
        }

        if (viewport.Width <= 0 || viewport.Height <= 0) {
            return false;
        }

        TextureDesc colorDesc;
        colorDesc.Width  = static_cast<uint32_t>(viewport.Width);
        colorDesc.Height = static_cast<uint32_t>(viewport.Height);
        colorDesc.Layers = 1;
        colorDesc.Format = TextureFormat::RGBA16F;
        colorDesc.Filter = TextureFilter::Linear;
        colorDesc.Wrap   = TextureWrap::ClampToEdge;

        TextureDesc depthDesc;
        depthDesc.Width  = static_cast<uint32_t>(viewport.Width);
        depthDesc.Height = static_cast<uint32_t>(viewport.Height);
        depthDesc.Layers = 1;
        depthDesc.Format = TextureFormat::Depth24;
        depthDesc.Filter = TextureFilter::Nearest;
        depthDesc.Wrap   = TextureWrap::ClampToEdge;

        mHdrColorTexture = device.CreateTexture2D(colorDesc);
        mHdrDepthTexture = device.CreateTexture2D(depthDesc);
        mHdrFramebuffer  = device.CreateFramebuffer(colorDesc);

        mHdrFramebuffer->SetColorAttachment(0, mHdrColorTexture);
        mHdrFramebuffer->SetDepthAttachment(mHdrDepthTexture);

        if (!mHdrFramebuffer->IsComplete()) {
            GOLIAS_LOG_ERROR("HDR framebuffer is incomplete.");
            return false;
        }

        if (!EnsureLdr(viewport)) {
            return false;
        }

        mHdrViewport = viewport;
        return true;
    }

    bool RenderTargets::EnsureLdr(const Viewport& viewport) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (mLdrFramebuffer && viewport.Width == mLdrViewport.Width && viewport.Height == mLdrViewport.Height) {
            return true;
        }

        if (viewport.Width <= 0 || viewport.Height <= 0) {
            return false;
        }

        // LDR intermediate: tonemapped result is written here before FXAA reads it.
        TextureDesc ldrDesc;
        ldrDesc.Width  = static_cast<uint32_t>(viewport.Width);
        ldrDesc.Height = static_cast<uint32_t>(viewport.Height);
        ldrDesc.Layers = 1;
        ldrDesc.Format = TextureFormat::RGBA8;
        ldrDesc.Filter = TextureFilter::Linear;
        ldrDesc.Wrap   = TextureWrap::ClampToEdge;

        mLdrColorTexture = device.CreateTexture2D(ldrDesc);
        mLdrFramebuffer  = device.CreateFramebuffer(ldrDesc);
        mLdrFramebuffer->SetColorAttachment(0, mLdrColorTexture);

        if (!mLdrFramebuffer->IsComplete()) {
            GOLIAS_LOG_ERROR("LDR framebuffer is incomplete.");
            return false;
        }

        mLdrViewport = viewport;
        return true;
    }

    bool RenderTargets::EnsureShadow(uint32_t resolution, TextureFormat depthFormat) {
        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        if (mShadowTexture && mShadowFramebuffer && resolution == mShadowResolution) {
            return true;
        }

        TextureDesc desc;
        desc.Width  = resolution;
        desc.Height = resolution;
        desc.Layers = CascadedShadowMapDesc::kMaxCascades;
        desc.Format = depthFormat;
        desc.Filter = TextureFilter::Nearest;
        desc.Wrap   = TextureWrap::ClampToBorder;

        if (!mShadowTexture) {
            mShadowTexture     = device.CreateTexture2DArray(desc);
            mShadowFramebuffer = device.CreateFramebuffer(desc);
        } else {
            mShadowTexture->Recreate(desc);
            mShadowFramebuffer = device.CreateFramebuffer(desc);
        }

        mShadowResolution = resolution;
        return true;
    }

} // namespace golias
