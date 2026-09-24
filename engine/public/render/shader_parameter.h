#pragma once
#include "graphics/buffer.h"
#include "graphics/graphics_device.h"

namespace golias {

    /// @brief  A single named GPU uniform block, lazily created and rebound on every Update().
    template <typename T, uint32_t Binding>
    class ShaderParameter {
    public:
        void Update(GraphicsDevice& device, const T& value) {
            EnsureBuffer(device);
            mBuffer->Update(&value, sizeof(T));
            mBuffer->Bind(Binding);
        }

        /// @brief  Overwrites part of the buffer (e.g. a single field) without touching the rest, then rebinds.
        void UpdatePartial(GraphicsDevice& device, const void* data, size_t size, size_t offset) {
            EnsureBuffer(device);
            mBuffer->Update(data, static_cast<uint32_t>(size), static_cast<uint32_t>(offset));
            mBuffer->Bind(Binding);
        }

    private:
        void EnsureBuffer(GraphicsDevice& device) {
            if (!mBuffer) {
                BufferDesc desc = {.Target = BufferTarget::Uniform, .Usage = BufferUsage::Dynamic, .Size = sizeof(T)};
                mBuffer         = device.CreateBuffer(desc);
            }
        }

        Ref<Buffer> mBuffer = nullptr;
    };

    using LightingParameter    = ShaderParameter<GpuLighting, GpuLayout::LightingBinding>;
    using FrameParameter       = ShaderParameter<GpuFrame, GpuLayout::FrameBinding>;
    using ObjectParameter      = ShaderParameter<GpuObject, GpuLayout::ObjectBinding>;
    using MaterialParameter    = ShaderParameter<GpuMaterial, GpuLayout::MaterialBinding>;
    using PostProcessParameter = ShaderParameter<GpuPostProcess, GpuLayout::PostProcessBinding>;
    using SkyboxParameter      = ShaderParameter<GpuSkybox, GpuLayout::SkyboxBinding>;

} // namespace golias
