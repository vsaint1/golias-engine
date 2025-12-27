#pragma once

#include "core/graphics/mesh.h"
#include "core/graphics/gles3/shaders/gl_shader.h"


namespace golias {

    class OpenglMesh final : public Mesh {
    public:
        OpenglMesh(const std::string_view pPath);
        
        OpenglMesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<Uint32>& indices);

        OpenglMesh(const VertexLayout& layout, const std::vector<float>& vertices);

        void Bind() override;

        void Draw() override;

        virtual ~OpenglMesh() override;
    private:
        GLuint VAO = 0;
        Buffer vertex_buffer;
        Buffer index_buffer;
    };


}; // namespace golias
