#pragma once
#include <vector>

namespace golias {

    struct VertexElement {
        uint32_t Index; // Attribute
        uint32_t Size; // Size in bytes (Number of Components)
        uint32_t Type; // Data type (e.g GL_FLOAT, GL_INT, etc.)
        uint32_t Offset; // Offset in bytes from the start of the vertex
    };

    struct VertexLayout {
        std::vector<VertexElement> Elements;
        uint32_t Stride = 0; // Total size of the vertex in bytes
    };
    
} // namespace golias
