#pragma once
#include "graphics/buffer.h"
#include "graphics/vertex_layout.h"
#include "stdafx.h"

namespace golias {

    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        void SetVertexBuffer(const Ref<Buffer>& vertexBuffer, const VertexLayout& layout);

        void SetIndexBuffer(const Ref<Buffer>& indexBuffer);

        struct InstanceAttribute {
            uint32_t Location       = 0;
            uint32_t ComponentCount = 4;
            uint32_t Offset         = 0;
        };

        void SetInstanceBuffer(const Ref<Buffer>& instanceBuffer, uint32_t stride, const std::vector<InstanceAttribute>& attributes);

        void Bind() const;
        void Unbind() const;

        void Draw(uint32_t vertexCount, uint32_t indexCount) const;

        void DrawIndexed(uint32_t start, uint32_t count) const;

        void DrawInstanced(uint32_t instanceCount, uint32_t vertexCount, uint32_t indexCount) const;


    private:
        VertexArray(const VertexArray&)            = delete;
        VertexArray& operator=(const VertexArray&) = delete;
        VertexArray(VertexArray&&)                 = delete;
        VertexArray& operator=(VertexArray&&)      = delete;

    private:
        uint32_t mHandle = 0;
    };

} // namespace golias
