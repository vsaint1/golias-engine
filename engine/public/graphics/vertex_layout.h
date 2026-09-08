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

    namespace VertexAttributeBinding {
        inline constexpr uint32_t Position       = 0;
        inline constexpr uint32_t Color          = 1;
        inline constexpr uint32_t TexCoord       = 2;
        inline constexpr uint32_t Normal         = 3;
        inline constexpr uint32_t Tangent        = 4;
        inline constexpr uint32_t Bitangent      = 5;
        inline constexpr uint32_t Joints         = 6;
        inline constexpr uint32_t Weights        = 7;
        inline constexpr uint32_t InstanceMatrix = 8; // 4 consecutive attributes (8..11)
        inline constexpr uint32_t InstanceColor  = 12;
    } // namespace VertexAttributeBinding

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

    enum class VertexFormat : uint8_t {
        Float, // 1 float
        Float2, // 2 floats
        Float3, // 3 floats
        Float4, // 4 floats
        UShort, // 1 unsigned short (floats when normalized)
        UShort4, // 4 unsigned shorts (integer attribute) - e.g. joint indices
        Int, // 1 signed int (integer attribute)
        Int4, // 4 signed ints (integer attribute)
        UByte4, // 4 unsigned bytes (integer attribute)
        UByte, // 1 unsigned byte (integer attribute)
    };

    /// @brief  Component count for a vertex format.
    inline constexpr uint32_t VertexFormatComponentCount(VertexFormat format) {
        switch (format) {
        case VertexFormat::Float2:
            return 2;
        case VertexFormat::Float3:
            return 3;
        case VertexFormat::Float4:
            return 4;
        case VertexFormat::UShort4:
            return 4;
        case VertexFormat::Int4:
            return 4;
        case VertexFormat::UByte4:
            return 4;
        default:
            return 1;
        }
    }

    struct VertexElement {
        uint32_t Index; // Attribute
        // uint32_t Size; // Size in bytes (Number of Components) @deprecated
        VertexFormat Format; // Data type (e.g. Float3, UShort4, etc.)
        uint32_t Offset; // Offset in bytes from the start of the vertex
    };

    struct VertexLayout {
        std::vector<VertexElement> Elements;
        uint32_t Stride = 0; // Total size of the vertex in bytes
    };

    /// @brief  Skinned vertex layout for the GPU buffer.
    inline VertexLayout SkinnedVertexLayout() {
        VertexLayout layout;
        layout.Elements = {
            {VertexAttributeBinding::Position,  VertexFormat::Float3,  offsetof(SkinnedVertex, position) },
            {VertexAttributeBinding::Color,     VertexFormat::Float3,  offsetof(SkinnedVertex, color)    },
            {VertexAttributeBinding::TexCoord,  VertexFormat::Float2,  offsetof(SkinnedVertex, texcoord) },
            {VertexAttributeBinding::Normal,    VertexFormat::Float3,  offsetof(SkinnedVertex, normal)   },
            {VertexAttributeBinding::Tangent,   VertexFormat::Float3,  offsetof(SkinnedVertex, tangent)  },
            {VertexAttributeBinding::Bitangent, VertexFormat::Float3,  offsetof(SkinnedVertex, bitangent)},
            {VertexAttributeBinding::Joints,    VertexFormat::UShort4, offsetof(SkinnedVertex, joints)   },
            {VertexAttributeBinding::Weights,   VertexFormat::Float4,  offsetof(SkinnedVertex, weights)  },
        };
        layout.Stride = sizeof(SkinnedVertex);

        return layout;
    }

    /// @brief  Builds the standard interleaved vertex layout.
    inline VertexLayout StandardVertexLayout() {
        VertexLayout layout;
        layout.Elements = {
            {VertexAttributeBinding::Position,  VertexFormat::Float3, offsetof(Vertex, position) },
            {VertexAttributeBinding::Color,     VertexFormat::Float3, offsetof(Vertex, color)    },
            {VertexAttributeBinding::TexCoord,  VertexFormat::Float2, offsetof(Vertex, texcoord) },
            {VertexAttributeBinding::Normal,    VertexFormat::Float3, offsetof(Vertex, normal)   },
            {VertexAttributeBinding::Tangent,   VertexFormat::Float3, offsetof(Vertex, tangent)  },
            {VertexAttributeBinding::Bitangent, VertexFormat::Float3, offsetof(Vertex, bitangent)},
        };
        layout.Stride = sizeof(Vertex);

        return layout;
    }

} // namespace golias
