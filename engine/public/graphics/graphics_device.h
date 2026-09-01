#pragma once
#include "graphics/gpu_types.h"
#include "graphics/render_types.h"

namespace golias {

    class Shader;
    class Material;
    class Mesh;
    class Texture2D;
    class Texture2DArray;
    class TextureCube;
    class Framebuffer;
    class Buffer;

    struct TextureDesc;

    class GraphicsDevice {

    public:
        ~GraphicsDevice() = default;

        bool Initialize();

        Ref<Shader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource);

        Ref<Texture2D> CreateTexture2D(const TextureDesc& desc);
        Ref<Texture2DArray> CreateTexture2DArray(const TextureDesc& desc);
        Ref<TextureCube> CreateTextureCube(const TextureDesc& desc);

        Ref<Framebuffer> CreateFramebuffer(const TextureDesc& desc);

        Ref<Buffer> CreateBuffer(const BufferDesc& desc);

        void SetClearColor(const Color& color = {0.25f, 0.45f, 0.75f, 1.0f});

        void SetDepthTestEnabled(bool enabled);

        void SetDepthWriteEnabled(bool enabled);

        void SetBlendMode(BlendMode mode = BlendMode::None);

        void ClearBuffers(ClearFlag flag = ClearFlag::Color | ClearFlag::Depth);

        const Viewport& GetViewport() const;
        void SetViewport(const Viewport& viewport);

        void SetCullMode(CullMode mode = CullMode::None);

        void BindShader(Shader* shader);

        void BindMaterial(Material* material);

        void BindMesh(Mesh* mesh);
        void DrawMesh(Mesh* mesh);
        void UnbindMesh(Mesh* mesh);

        void BindBuffer(Buffer* buffer);
        void UnbindBuffer(Buffer* buffer);

    private:
        std::unordered_map<std::string, Ref<Shader>> mShaderCache = {};
        Viewport mViewport                                        = {};
    };
} // namespace golias
