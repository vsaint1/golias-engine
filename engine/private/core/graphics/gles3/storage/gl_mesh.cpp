#include "core/graphics/gles3/storage/gl_mesh.h"

#include "core/engine.h"
#include "core/graphics/gles3/gl_common.h"

namespace golias {


    OpenglMesh::OpenglMesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
        : Mesh(layout, vertices, indices) {
        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        vertex_buffer = rd->CreateGPUBuffer(
            vertices.size() * sizeof(float), vertices.data(), EBufferUsageFlags::DYNAMIC_DRAW, EBufferTarget::ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.handle);

        if (!indices.empty()) {
            index_buffer = rd->CreateGPUBuffer(
                indices.size() * sizeof(uint32_t), indices.data(), EBufferUsageFlags::DYNAMIC_DRAW, EBufferTarget::ELEMENT_ARRAY_BUFFER);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.handle);
        }

        for (const auto& e : layout.elements) {
            glEnableVertexAttribArray(e.location);
            glVertexAttribPointer(e.location,
                                  e.components,
                                  ToGLDataType(e.type),
                                  e.normalized ? GL_TRUE : GL_FALSE,
                                  layout.stride,
                                  reinterpret_cast<void*>(static_cast<uintptr_t>(e.offset)));
        }

        glBindVertexArray(0);

        spdlog::info("OpenglMesh::OpenglMesh Created ({} Vertices, {} Indices)", GetVertexCount(), GetIndexCount());
    }


    OpenglMesh::OpenglMesh(const VertexLayout& layout, const std::vector<float>& vertices) : Mesh(layout, vertices) {
        auto rd = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        vertex_buffer = rd->CreateGPUBuffer(
            vertices.size() * sizeof(float), vertices.data(), EBufferUsageFlags::STATIC_DRAW, EBufferTarget::ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.handle);

        for (const auto& e : layout.elements) {
            glEnableVertexAttribArray(e.location);
            glVertexAttribPointer(e.location,
                                  e.components,
                                  ToGLDataType(e.type),
                                  e.normalized ? GL_TRUE : GL_FALSE,
                                  layout.stride,
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

    void OpenglMesh::DrawIndexed(Uint32 startIndex, Uint32 indexCount) {
        if (indexCount == 0) {
            return;
        }

        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(indexCount),
                       GL_UNSIGNED_INT,
                       reinterpret_cast<void*>(static_cast<size_t>(startIndex * sizeof(Uint32))));
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

    void OpenglMesh::Update(const std::vector<float>& vertices, const std::vector<Uint32>& indices) {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Uint32), indices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        vertex_count = vertices.size() / (vertex_layout.stride / sizeof(float));

        if (index_buffer.handle == 0) {
            auto rd      = Engine::GetInstance().GetSceneRenderer().GetRenderingDevice();
            index_buffer = rd->CreateGPUBuffer(
                indices.size() * sizeof(uint32_t), indices.data(), EBufferUsageFlags::DYNAMIC_DRAW, EBufferTarget::ELEMENT_ARRAY_BUFFER);
        } else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.handle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Uint32), indices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        index_count = indices.size();
    }

    void OpenglMesh::Update(const std::vector<float>& vertices) {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.handle);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        vertex_count = vertices.size() / (vertex_layout.stride / sizeof(float));
    }

} // namespace golias
