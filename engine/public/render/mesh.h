#pragma once
#include "graphics/vertex_layout.h"
#include "stdafx.h"

namespace golias {


    class Mesh {
    public:

        Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
        Mesh(const VertexLayout& layout, const std::vector<float>& vertices);
        
        ~Mesh();

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
