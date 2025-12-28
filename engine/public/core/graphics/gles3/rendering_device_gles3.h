#pragma once

#include "core/graphics/gles3/shaders/gl_shader.h"
#include "core/graphics/gles3/storage/gl_mesh.h"
#include "core/graphics/gles3/storage/gl_texture_2d.h"
#include "core/graphics/rendering_device.h"


namespace golias {


    class RenderingDeviceGLES3 final : public RenderingDevice {
    public:
        RenderingDeviceGLES3() = default;

        ~RenderingDeviceGLES3();

        bool Initialize(SDL_Window* sdl_window) override;

        void BindShader(Shader* shader);
        void BindMesh(Mesh* mesh);
        void BindMaterial(Material* material);

        std::shared_ptr<Shader> GetDefaultShader3D() const;
        
        std::shared_ptr<Texture2D> CreateTextureFromFile(const std::string_view pFilePath) override;
        std::shared_ptr<Texture2D> CreateTextureFromData(int w, int h, ETextureFormat format, const Uint8* data) override;

        std::shared_ptr<Shader> CreateShaderFromFile(const std::string_view pFilePath) override;
        std::shared_ptr<Shader> CreateShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) override;


        std::shared_ptr<Mesh> CreateMesh() override;
        std::shared_ptr<Mesh> CreateMeshFromData(const VertexLayout& layout, const std::vector<float>& vertices,
                                                 const std::vector<uint32_t>& indices) override;

        Buffer CreateGPUBuffer(size_t size, const void* data, EBufferUsageFlags bufferFlags, EBufferTarget bufferTarget) override;

        void Clear(glm::vec4 color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f)) override;

        void DrawMesh(Mesh* mesh);

        void Present() override;

    private:
        SDL_GLContext gl_context = nullptr;

        std::shared_ptr<Shader> default_shader_3d = nullptr;
        bool CreateDefaultShaders() ;

    };
}; // namespace golias
