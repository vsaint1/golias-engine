#pragma once
#include <SDL3/SDL_stdinc.h>
#include <vector>

#include <glm/glm.hpp>


enum class EBufferTarget : uint32_t {
    ARRAY_BUFFER         = 0x1, // GL_ARRAY_BUFFER
    ELEMENT_ARRAY_BUFFER = 0x2 // GL_ELEMENT_ARRAY_BUFFER
};

enum class EBufferUsageFlags : uint32_t {
    STATIC_DRAW  = 0x1, // GL_STATIC_DRAW
    DYNAMIC_DRAW = 0x2, // GL_DYNAMIC_DRAW
    STREAM_DRAW  = 0x3 // GL_STREAM_DRAW
};

struct Buffer {
    uint32_t handle = 0;
    size_t size     = 0;
    EBufferUsageFlags usage_flags;
    EBufferTarget target = EBufferTarget::ARRAY_BUFFER;
};


enum class EDataType : uint32_t {
    FLOAT = 0x1, // GL_FLOAT 
    INT   = 0x2, // GL_INT
    SHORT = 0x3 // GL_SHORT
};

namespace golias {

    struct VertexElement {
        Uint32 location; // Shader layout(location = X)
        Uint32 components; // Number of components (1..4)
        EDataType type; // GL_FLOAT, GL_INT, etc.
        bool normalized; // GL_TRUE / GL_FALSE
        Uint32 offset; // Byte offset in vertex
    };

    struct VertexLayout {
        std::vector<VertexElement> elements;
        Uint32 stride = 0; // Total vertex size in bytes
    };

}; // namespace golias
