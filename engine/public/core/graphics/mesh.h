#pragma once

#include "structs.h"


namespace golias {

    class Mesh {
    public:

        Mesh() = default;

        Mesh(const VertexLayout& layout, const std::vector<float>& vertices, const std::vector<Uint32>& indices);
        
        Mesh(const VertexLayout& layout, const std::vector<float>& vertices);

        virtual void Bind() = 0;

        virtual void Draw() = 0;

        void SetVertexLayout(const VertexLayout& layout);

        const VertexLayout& GetVertexLayout() const;

        void SetVertexCount(size_t count);

        size_t GetVertexCount() const;

        void SetIndexCount(size_t count);

        size_t GetIndexCount() const;

        virtual ~Mesh() = default;

    private:
        Mesh(const Mesh&)            = delete;
        Mesh& operator=(const Mesh&) = delete;

    protected:
        VertexLayout vertex_layout;
        size_t vertex_count = 0;
        size_t index_count  = 0;

    };

}; // namespace golias
