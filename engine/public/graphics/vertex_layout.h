#pragma once
#include "stdafx.h"

namespace golias {


    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texcoord;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };

    namespace VertexAttrib {
        inline constexpr uint32_t Position  = 0;
        inline constexpr uint32_t Color     = 1;
        inline constexpr uint32_t TexCoord  = 2;
        inline constexpr uint32_t Normal    = 3;
        inline constexpr uint32_t Tangent   = 4;
        inline constexpr uint32_t Bitangent = 5;
    } // namespace VertexAttrib

    inline constexpr uint32_t kVertexFloatCount = static_cast<uint32_t>(sizeof(Vertex) / sizeof(float));

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

    /// @brief  Builds the standard interleaved vertex layout.
    inline VertexLayout StandardVertexLayout() {
        VertexLayout layout;
        layout.Stride   = sizeof(Vertex);
        layout.Elements = {
            {VertexAttrib::Position,  3, GL_FLOAT, offsetof(Vertex, position) },
            {VertexAttrib::Color,     3, GL_FLOAT, offsetof(Vertex, color)    },
            {VertexAttrib::TexCoord,  2, GL_FLOAT, offsetof(Vertex, texcoord) },
            {VertexAttrib::Normal,    3, GL_FLOAT, offsetof(Vertex, normal)   },
            {VertexAttrib::Tangent,   3, GL_FLOAT, offsetof(Vertex, tangent)  },
            {VertexAttrib::Bitangent, 3, GL_FLOAT, offsetof(Vertex, bitangent)},
        };
        return layout;
    }

} // namespace golias
