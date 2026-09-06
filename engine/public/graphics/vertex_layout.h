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


    struct SkinnedVertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texcoord;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        uint16_t joints[4]; // Attribute 6: joint indices (GL_UNSIGNED_SHORT)
        glm::vec4 weights; // Attribute 7: joint weights
    };

    namespace VertexAttrib {
        inline constexpr uint32_t Position  = 0;
        inline constexpr uint32_t Color     = 1;
        inline constexpr uint32_t TexCoord  = 2;
        inline constexpr uint32_t Normal    = 3;
        inline constexpr uint32_t Tangent   = 4;
        inline constexpr uint32_t Bitangent = 5;
        inline constexpr uint32_t Joints    = 6;
        inline constexpr uint32_t Weights   = 7;
    } // namespace VertexAttrib

    inline constexpr uint32_t kVertexFloatCount = static_cast<uint32_t>(sizeof(Vertex) / sizeof(float));

    namespace VertexAttributeOffsets {
        inline constexpr uint32_t Position                 = 0;
        inline constexpr uint32_t Color                    = 3;
        inline constexpr uint32_t TexCoord                 = 6;
        inline constexpr uint32_t Normal                   = 8;
        inline constexpr uint32_t Tangent                  = 11;
        inline constexpr uint32_t Bitangent                = 14;
        inline constexpr uint32_t Joints                   = 17;
        inline constexpr uint32_t Weights                  = 21;
        inline constexpr uint32_t kSkinnedVertexFloatCount = 25;
    } // namespace VertexAttributeOffsets

    struct VertexElement {
        uint32_t Index; // Attribute
        uint32_t Size; // Size in bytes (Number of Components)
        uint32_t Type; // Data type (e.g GL_FLOAT, GL_INT, etc.)
        uint32_t Offset; // Offset in bytes from the start of the vertex
        bool Integer = false; // Uses glVertexAttribIPointer (integer vertex attributes, no normalization)
    };

    struct VertexLayout {
        std::vector<VertexElement> Elements;
        uint32_t Stride = 0; // Total size of the vertex in bytes
    };

    /// @brief  Skinned vertex layout for the GPU buffer.
    inline VertexLayout SkinnedVertexLayout() {
        VertexLayout layout;
        layout.Stride   = 92; // base(17 floats) + joints(ushort x4) + weights(4 floats)
        layout.Elements = {
            {VertexAttrib::Position,  3, GL_FLOAT,          0,  false},
            {VertexAttrib::Color,     3, GL_FLOAT,          12, false},
            {VertexAttrib::TexCoord,  2, GL_FLOAT,          24, false},
            {VertexAttrib::Normal,    3, GL_FLOAT,          32, false},
            {VertexAttrib::Tangent,   3, GL_FLOAT,          44, false},
            {VertexAttrib::Bitangent, 3, GL_FLOAT,          56, false},
            {VertexAttrib::Joints,    4, GL_UNSIGNED_SHORT, 68, true },
            {VertexAttrib::Weights,   4, GL_FLOAT,          76, false},
        };
        return layout;
    }

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
