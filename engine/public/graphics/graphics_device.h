#pragma once
#include "stdafx.h"

namespace golias {

    class Shader;
    class Material;

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
        Ref<Shader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource);

        GLuint CreateVertexBuffer(const std::vector<float>& vertices);

        GLuint CreateIndexBuffer(const std::vector<uint32_t>& indices);

        void SetClearColor(const Color& color = {0.25f, 0.45f, 0.75f, 1.0f});

        void ClearBuffers(GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        void SetViewport(const Viewport& viewport);

        void BindShader(Shader* shader);

        void BindMaterial(Material* material);
    };
} // namespace golias
