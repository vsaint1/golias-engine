#pragma once
#include <SDL3/SDL_stdinc.h>
#include <vector>

#include <glm/glm.hpp>


enum class EBufferTarget : uint32_t {
    ARRAY_BUFFER         = 0x8892, // GL_ARRAY_BUFFER
    ELEMENT_ARRAY_BUFFER = 0x8893 // GL_ELEMENT_ARRAY_BUFFER
};

enum class EBufferUsageFlags : uint32_t {
    STATIC_DRAW  = 0x88E4, // GL_STATIC_DRAW
    DYNAMIC_DRAW = 0x88E8, // GL_DYNAMIC_DRAW
    STREAM_DRAW  = 0x88E0 // GL_STREAM_DRAW
};

struct Buffer {
    uint32_t handle = 0;
    size_t size     = 0;
    EBufferUsageFlags usage_flags;
    EBufferTarget target = EBufferTarget::ARRAY_BUFFER;
};


enum class DataType : uint32_t {
    FLOAT = 0x1406, // GL_FLOAT
    INT   = 0x1404 // GL_INT
};

namespace golias {

    struct VertexElement {
        Uint32 location; // Shader layout(location = X)
        Uint32 components; // Number of components (1..4)
        Uint32 type; // GL_FLOAT, GL_INT, etc.
        bool normalized; // GL_TRUE / GL_FALSE
        Uint32 offset; // Byte offset in vertex
    };

    struct VertexLayout {
        std::vector<VertexElement> elements;
        Uint32 stride = 0; // Total vertex size in bytes
    };

}; // namespace golias
