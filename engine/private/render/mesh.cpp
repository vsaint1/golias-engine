#include "render/mesh.h"

#include "core/engine.h"

namespace golias {


    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices) {
        mVertexLayout = layout;

        mVertexCount = vertices.size() / (layout.Stride / sizeof(float));
        mIndexCount  = indices.size();

        GraphicsDevice& device = Engine::GetInstance().GetGraphicsDevice();

        mVBO = device.CreateVertexBuffer(vertices);

        if (!indices.empty()) {
            mEBO = device.CreateIndexBuffer(indices);
        }

        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);

        for (const auto& element : layout.Elements) {
            glVertexAttribPointer(element.Index, element.Size, element.Type, GL_FALSE, layout.Stride, (void*) (uintptr_t) (element.Offset));
            glEnableVertexAttribArray(element.Index);
        }

        if (mIndexCount > 0 && mEBO != 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices) : Mesh(layout, vertices, std::vector<uint32_t>()) {
    }

    void Mesh::Bind() const {
        glBindVertexArray(mVAO);
    }

    void Mesh::Draw() const {

        if (mIndexCount > 0) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mVertexCount));
        }
    }

    Mesh::~Mesh() {

        if (mVAO) {
            glDeleteVertexArrays(1, &mVAO);
        }

        if (mVBO) {
            glDeleteBuffers(1, &mVBO);
        }

        if (mEBO) {
            glDeleteBuffers(1, &mEBO);
        }
    }


} // namespace golias
