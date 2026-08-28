#pragma once
#include "graphics/gpu_types.h"
#include "graphics/render_types.h"

namespace golias {

    class Shader;
    class Material;
    class Mesh;
    class Texture2DArray;
    class TextureCube;
    class Framebuffer;
    struct TextureDesc;

    class GraphicsDevice {

    public:
        ~GraphicsDevice();

        bool Initialize();

        Ref<Shader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource);

        Ref<Texture2DArray> CreateTexture2DArray(const TextureDesc& desc);
        Ref<TextureCube> CreateTextureCube(const TextureDesc& desc);

        Ref<Framebuffer> CreateFramebuffer(const TextureDesc& desc);

        GLuint CreateVertexBuffer(const std::vector<float>& vertices);

        GLuint CreateIndexBuffer(const std::vector<uint32_t>& indices);

        GLuint CreateUniformBuffer(size_t size);

        void SetClearColor(const Color& color = {0.25f, 0.45f, 0.75f, 1.0f});

        void SetDepthTestEnabled(bool enabled);
        void SetDepthWriteEnabled(bool enabled);

        void ClearBuffers(ClearFlag flag = ClearFlag::Color | ClearFlag::Depth);

        void SetViewport(const Viewport& viewport);

        void SetCullMode(CullMode mode = CullMode::None);

        void BindShader(Shader* shader);

        void BindMaterial(Material* material);

        void BindMesh(Mesh* mesh);

        void DrawMesh(Mesh* mesh);

        void UnbindMesh(Mesh* mesh);

        void UpdateUniformBuffer(GLuint buffer, const void* data, size_t size, size_t offset = 0);

        void BindUniformBuffer(GLuint buffer, uint32_t binding);

        void DestroyBuffer(GLuint buffer);

    private:
        std::unordered_map<std::string, Ref<Shader>> mShaderCache = {};
    };
} // namespace golias
