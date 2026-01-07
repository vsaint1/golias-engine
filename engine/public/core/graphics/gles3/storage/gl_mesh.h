#pragma once

#include "core/graphics/mesh.h"
#include "core/graphics/gles3/shaders/gl_shader.h"


namespace golias {

    class OpenglMesh final : public Mesh {
    public:
        
        OpenglMesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<Uint32>& indices);

        OpenglMesh(const VertexLayout& layout, const std::vector<float>& vertices);

        void Bind() override;
        void Draw() override;
        void DrawIndexed(Uint32 startIndex, Uint32 indexCount) override;
        void Unbind() override;
        void Update(const std::vector<float>& vertices, const std::vector<Uint32>& indices) override;
        void Update(const std::vector<float>& vertices) override;
        
        virtual ~OpenglMesh() override;
    private:
        GLuint VAO = 0;
        Buffer vertex_buffer;
        Buffer index_buffer;
    };


}; // namespace golias
