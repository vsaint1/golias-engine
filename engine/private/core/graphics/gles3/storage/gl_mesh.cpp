#include "core/graphics/gles3/storage/gl_mesh.h"

#include "core/engine.h"

#include "core/graphics/gles3/gl_common.h"

namespace golias {



    OpenglMesh::OpenglMesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
        : Mesh(layout, vertices, indices) {
        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        vertex_buffer = rd->CreateGPUBuffer(vertices.size() * sizeof(float), vertices.data(), EBufferUsageFlags::STATIC_DRAW,
                                           EBufferTarget::ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.handle);

        if (!indices.empty()) {
            index_buffer = rd->CreateGPUBuffer(indices.size() * sizeof(uint32_t), indices.data(), EBufferUsageFlags::STATIC_DRAW,
                                              EBufferTarget::ELEMENT_ARRAY_BUFFER);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.handle);
        }

        for (const auto& e : layout.elements) {
            glEnableVertexAttribArray(e.location);
            glVertexAttribPointer(e.location, e.components, ToGLDataType(e.type), e.normalized ? GL_TRUE : GL_FALSE, layout.stride,
                                  reinterpret_cast<void*>(static_cast<uintptr_t>(e.offset)));
        }

        glBindVertexArray(0);

        spdlog::info("OpenglMesh::OpenglMesh Created ({} Vertices, {} Indices)", GetVertexCount(), GetIndexCount());
    }


    OpenglMesh::OpenglMesh(const VertexLayout& layout, const std::vector<float>& vertices) : Mesh(layout, vertices) {
        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        vertex_buffer = rd->CreateGPUBuffer(vertices.size() * sizeof(float), vertices.data(), EBufferUsageFlags::STATIC_DRAW,
                                           EBufferTarget::ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.handle);

        for (const auto& e : layout.elements) {
            glEnableVertexAttribArray(e.location);
            glVertexAttribPointer(e.location, e.components, ToGLDataType(e.type), e.normalized ? GL_TRUE : GL_FALSE, layout.stride,
                                  reinterpret_cast<void*>(static_cast<uintptr_t>(e.offset)));
        }

        glBindVertexArray(0);

        spdlog::info("OpenglMesh::OpenglMesh Created ({} Vertices)", GetVertexCount());
    }

    void OpenglMesh::Bind() {

        glBindVertexArray(VAO);
    }

    void OpenglMesh::Unbind() {

        glBindVertexArray(0);
    }
    
    void OpenglMesh::Draw() {

        if (GetIndexCount() > 0) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(GetIndexCount()), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(GetVertexCount()));
        }
    }


    OpenglMesh::~OpenglMesh() {

        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }

        if (vertex_buffer.handle != 0) {
            glDeleteBuffers(1, &vertex_buffer.handle);
            vertex_buffer.handle = 0;
        }

        if (index_buffer.handle != 0) {
            glDeleteBuffers(1, &index_buffer.handle);
            index_buffer.handle = 0;
        }

    }


} // namespace golias
