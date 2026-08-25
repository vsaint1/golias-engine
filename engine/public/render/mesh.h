#pragma once
#include "graphics/vertex_layout.h"
#include "stdafx.h"

namespace golias {

    class Model;
    struct ModelPrimitive;

    class Mesh {
    public:

        Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
        Mesh(const VertexLayout& layout, const std::vector<float>& vertices);
        
        ~Mesh();

        static Ref<Mesh> Load(CString path);

        static Ref<Mesh> Create(const Model& model, const ModelPrimitive& primitive);

        static Ref<Mesh> CreateCube(const glm::vec3& size = glm::vec3(1.0f));

        void Bind() const;

        void Draw() const;

    private:
        Mesh(const Mesh&)            = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&)                 = delete;
        Mesh& operator=(Mesh&&)      = delete;

    private:
        VertexLayout mVertexLayout;
        size_t mVertexCount = 0;
        size_t mIndexCount  = 0;

        GLuint mVAO = 0;
        GLuint mVBO = 0;
        GLuint mEBO = 0;
    };

} // namespace golias
