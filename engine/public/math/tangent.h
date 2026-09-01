#pragma once

#include "stdafx.h"

namespace golias {


    /// @brief  Describes the location of tangent-relevant vertex attributes inside an interleaved float vertex buffer.
    struct TangentLayout {
        size_t Stride    = 0; // Floats per vertex
        size_t Position  = 0; // vec3 (3 floats)
        size_t TexCoord  = 6; // vec2 (2 floats)
        size_t Normal    = 8; // vec3 (3 floats)
        size_t Tangent   = 11; // vec3 (3 floats)
        size_t Bitangent = 14; // vec3 (3 floats)
    };

    /// @brief  Computes per-vertex tangent and bitangent vectors from geometry using the Lengyel method.
    //  https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
    void GenerateTangents(std::vector<float>& vertices,
                          const std::vector<uint32_t>& indices,
                          const TangentLayout& layout,
                          size_t vertexCount = 0);

} // namespace golias
