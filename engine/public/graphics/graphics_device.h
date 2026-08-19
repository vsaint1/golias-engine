#pragma once
#include "stdafx.h"

namespace golias {

    class Shader;
    class Material;

    struct Color {
        float r, g, b, a;

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

        void BindShader(Shader* shader);

        void BindMaterial(Material* material);
    };
} // namespace golias
