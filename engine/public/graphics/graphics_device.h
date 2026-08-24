#pragma once
#include "stdafx.h"

namespace golias {

    class Shader;
    class Material;
    constexpr size_t kMaxLights = 32;

    struct alignas(16) GpuLight {
        glm::vec4 Position;
        glm::vec4 Direction;
        glm::vec4 ColorIntensity;
        float Range;
        float SpotAngle;
        int Type;
        int IsShadowCaster;
    };

    static_assert(sizeof(GpuLight) == 64, "GpuLight must match the std140 light layout");

    struct alignas(16) GpuLighting {
        int Count;
        int Padding0;
        int Padding1;
        int Padding2;
        GpuLight Lights[kMaxLights];
    };

    struct Viewport {
        int X = 0, Y = 0, Width = 0, Height = 0;
    };

    struct Color {
        float R = 0.0f, G = 0.0f, B = 0.0f, A = 1.0f;

        static Color White() {
            return {1.0f, 1.0f, 1.0f, 1.0f};
        }
    };

    class GraphicsDevice {

    public:
        ~GraphicsDevice();

        bool Initialize();

        Ref<Shader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource);

        GLuint CreateVertexBuffer(const std::vector<float>& vertices);

        GLuint CreateIndexBuffer(const std::vector<uint32_t>& indices);

        GLuint CreateUniformBuffer(size_t size);

        void SetClearColor(const Color& color = {0.25f, 0.45f, 0.75f, 1.0f});

        void ClearBuffers(GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        void SetViewport(const Viewport& viewport);

        void BindShader(Shader* shader);

        void BindMaterial(Material* material);

        void UpdateUniformBuffer(GLuint buffer, const void* data, size_t size, size_t offset = 0);

        void BindUniformBuffer(GLuint buffer, uint32_t binding);

        void DestroyBuffer(GLuint buffer);
    };
} // namespace golias
