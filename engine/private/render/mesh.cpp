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

    Ref<Mesh> Mesh::Create(const Model& model, const ModelPrimitive& primitive) {
        const size_t stride = model.GetVertexLayout().Stride / sizeof(float);
        const auto& vertices = model.GetVertices();
        const auto& indices = model.GetIndices();
        if (primitive.vertexOffset + primitive.vertexCount > vertices.size() / stride
            || primitive.indexOffset + primitive.indexCount > indices.size()) {
            return nullptr;
        }

        std::vector<float> primitiveVertices(
            vertices.begin() + primitive.vertexOffset * stride,
            vertices.begin() + (primitive.vertexOffset + primitive.vertexCount) * stride);
       
        std::vector<uint32_t> primitiveIndices;
        primitiveIndices.reserve(primitive.indexCount);
       
        for (size_t i = 0; i < primitive.indexCount; ++i) {
            primitiveIndices.push_back(indices[primitive.indexOffset + i] - static_cast<uint32_t>(primitive.vertexOffset));
        }

        return std::make_shared<Mesh>(model.GetVertexLayout(), primitiveVertices, primitiveIndices);
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

    Ref<Mesh> Mesh::CreateCube(const glm::vec3& size) {

        const glm::vec3 halfSize = size * 0.5f;
        
        std::vector<float> vertices = {
            // Front (+Z)
            halfSize.x,  halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
            -halfSize.x,  halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
            -halfSize.x, -halfSize.y, halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
            halfSize.x, -halfSize.y, halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,

            // Back (-Z)
            -halfSize.x,  halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
            halfSize.x,  halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
            halfSize.x, -halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
            -halfSize.x, -halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,

            // Top (+Y)
            -halfSize.x,  halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
            halfSize.x,  halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
            halfSize.x,  halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
            -halfSize.x,  halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,

            // Bottom (-Y)
            -halfSize.x, -halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
            halfSize.x, -halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
            halfSize.x, -halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
            -halfSize.x, -halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f,

            // Right (+X)
            halfSize.x,  halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
            halfSize.x,  halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
            halfSize.x, -halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
            halfSize.x, -halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,

            // Left (-X)
            -halfSize.x,  halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
            -halfSize.x,  halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
            -halfSize.x, -halfSize.y, -halfSize.z,       1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
            -halfSize.x, -halfSize.y,  halfSize.z,       1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f
        };

        std::vector<uint32_t> indices = {
            0,  1,  2,  0,  2,  3,       // Front
            4,  5,  6,  4,  6,  7,       // Back
            8,  9, 10,  8, 10, 11,       // Top
            12, 13, 14, 12, 14, 15,       // Bottom
            16, 17, 18, 16, 18, 19,       // Right
            20, 21, 22, 20, 22, 23        // Left
        };

        VertexLayout layout;
        layout.Elements.push_back({0, 3, GL_FLOAT, 0});                     // Position
        layout.Elements.push_back({1, 3, GL_FLOAT, 3 * sizeof(float)});    // Color
        layout.Elements.push_back({2, 2, GL_FLOAT, 6 * sizeof(float)});    // TexCoord
        layout.Elements.push_back({3, 3, GL_FLOAT, 8 * sizeof(float)});    // Normal
        layout.Stride = 11 * sizeof(float);

        Ref<Mesh> mesh = std::make_shared<Mesh>(layout, vertices, indices);

        return mesh;
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
