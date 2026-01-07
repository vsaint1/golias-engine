#pragma once

#include "structs.h"
#include <memory>

namespace golias {

    class Mesh {
    public:

        Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<Uint32>& indices);

        Mesh(const VertexLayout& layout, const std::vector<float>& vertices);

        static std::shared_ptr<Mesh> CreateBox(const glm::vec3& extents = glm::vec3(1.0f));
        static std::shared_ptr<Mesh> CreateSphere(float radius = 1.0f, uint32_t segments = 16, uint32_t rings = 16);
        static std::shared_ptr<Mesh> CreateQuad(float width = 1.0f, float height = 1.0f);

        virtual void Bind() = 0;
        virtual void Draw() = 0;
        virtual void DrawIndexed(uint32_t startIndex, uint32_t indexCount) = 0;
        virtual void Unbind() = 0;
        virtual void Update(const std::vector<float>& vertices, const std::vector<Uint32>& indices) = 0;
        virtual void Update(const std::vector<float>& vertices) = 0;

        void SetVertexLayout(const VertexLayout& layout);

        const VertexLayout& GetVertexLayout() const;

        void SetVertexCount(size_t count);

        size_t GetVertexCount() const;

        void SetIndexCount(size_t count);

        size_t GetIndexCount() const;

        EDataType GetIndexType() const;

        void SetIndexType(EDataType type);
    
        virtual ~Mesh() = default;

private:
    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;

protected:
    Mesh() = default;

    VertexLayout vertex_layout = VertexLayout();
    size_t vertex_count = 0;
    size_t index_count  = 0;

    // std::vector<float> vertices_data;
    // std::vector<Uint32> indices_data;
    EDataType index_type = EDataType::UNSIGNED_INT;

    

};
}
; // namespace golias
