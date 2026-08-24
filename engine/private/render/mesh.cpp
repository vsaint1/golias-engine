#include "render/mesh.h"

#include "core/engine.h"
#include "render/model.h"

namespace golias {

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<uint32_t>& indices) {
        mVertexLayout = layout;
        mVertexCount  = vertices.size() / (layout.Stride / sizeof(float));
        mIndexCount   = indices.size();

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

        if (mEBO) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    Mesh::Mesh(const VertexLayout& layout, const std::vector<float>& vertices) : Mesh(layout, vertices, {}) {
    }

    Ref<Mesh> Mesh::Load(CString path) {
        const Ref<Model> model = Model::Load(path);

        if (!model || model->GetVertices().empty() || model->GetIndices().empty()) {
            GOLIAS_LOG_ERROR("Failed to load mesh from model: %s", path.data());
            return nullptr;
        }

        Ref<Mesh> mesh = std::make_shared<Mesh>(model->GetVertexLayout(), model->GetVertices(), model->GetIndices());

        return mesh;
    }

    void Mesh::Bind() const {
        glBindVertexArray(mVAO);
    }

    void Mesh::Draw() const {
        if (mIndexCount) {
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
