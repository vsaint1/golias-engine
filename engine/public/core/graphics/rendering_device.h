#pragma once

#include "core/graphics/texture_2d.h"
#include "core/graphics/material.h"
#include "core/graphics/mesh.h"
#include "core/graphics/shader.h"
#include <SDL3/SDL.h>


namespace golias {

    enum ERenderingDeviceType {
        COMPATIBILITY,
        FORWARD_PLUS,
    };


    class RenderingDevice {
    public:
        RenderingDevice() = default;

        virtual ~RenderingDevice() = default;

        virtual bool Initialize(SDL_Window* sdl_window) = 0;

        virtual void BindShader(Shader* shader)       = 0;
        virtual void BindMesh(Mesh* mesh)             = 0;
        virtual void BindMaterial(Material* material) = 0;

        virtual std::shared_ptr<Texture2D> CreateTextureFromFile(const std::string_view pFilePath) = 0;
        virtual std::shared_ptr<Texture2D> CreateTextureFromData(int w, int h, ETextureFormat format, const Uint8* data) = 0;

        virtual std::shared_ptr<Shader> CreateShaderFromFile(const std::string_view pFilePath)                                     = 0;
        virtual std::shared_ptr<Shader> CreateShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) = 0;

        virtual Buffer CreateGPUBuffer(size_t size, const void* data, EBufferUsageFlags bufferFlags, EBufferTarget bufferTarget) = 0;

        virtual std::shared_ptr<Mesh> CreateMeshFromFile(const std::string_view pPath) = 0;
        virtual std::shared_ptr<Mesh> CreateMeshFromData(const VertexLayout& layout, const std::vector<float>& vertices,
                                                         const std::vector<uint32_t>& indices) = 0;

        virtual void Clear(glm::vec4 color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f)) = 0;

        virtual void DrawMesh(Mesh* mesh) = 0;

        virtual void Present() = 0;

    protected:
        glm::vec4 clear_color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
        SDL_Window* window    = nullptr;
    };
}; // namespace golias
